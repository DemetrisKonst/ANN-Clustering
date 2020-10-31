#include <random>

#include "../core/item.hpp"
#include "../LSH/LSHFun.hpp"
#include "../metrics/metrics.hpp"
#include "../interfaces/HC_interface.h"
#include "../utils/ANN.hpp"

/*
This class is responsible for mapping "HashFunction->HashVector()" values into {0,1}.
Once created, a random integer is initialized in range [min, max].
Whenever a "HashFunction->HashVector()" value is entered, the mapping is done like such:
return hVal >= randomNumber.
As long as min & max are carefully picked, it ensures a uniform distribution of 0s and 1s
*/
class FFunction {
private:
  unsigned int randomNumber;

public:
  FFunction (int min, int max) {
    // Initialize a random value between min & max
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(min, max);

    randomNumber = distribution(generator);
  }

  int map (unsigned int hVal) {
    return hVal >= randomNumber;
  }
};

template <typename T>
class Hypercube {
private:
  // # of images
  int imageCount;
  // the dimension of the Hypercube (denoted as d')
  int HCDimension;
  // the dimension of the features
  int dataDimension;
  // the window size inserted by the user, it is used inside HashFunction to shift the items
  int windowSize;

  // Array of f functions
  FFunction** mapFun;
  int fMin;
  int fMax;
  // Array of h functions
  HashFunction<T>** LSHFun;

  /*
  The data structure in which the items are stored.
  It is an array of vectors of items.
  The size of the array is 2^HCDimension.
  Each array index represents a vertex in the Hypercube
  (i.e. its decimal value calculated by its binary representation)
  */
  std::vector<Item<T>*>* H;

  unsigned int mConstant;
  int* m_mod_MValues;
  /*
  Calculate f(h(x)) for every dimension in the Hypercube.
  Then create an integer of HCDimension bits while shifting and inserting
  each value calculated
  */
  int _calculateVertex (T* x) {
    int vertex = 0;

    for (int i = 0; i < HCDimension; i++) {
      int hashRes = LSHFun[i]->HashVector(x);
      int mapRes = mapFun[i]->map(hashRes);

      if (mapRes == 1) {
        vertex |= 1;
      }

      // We don't want the last shift to take place
      if (i < HCDimension-1)
        vertex <<= 1;
    }

    return vertex;
  }

  /*
  The following function returns all vertex indices of distance "dist" from startingVertex
  */
  std::vector<int> _getProbesPerHD (int dist, int startingVertex) {
    std::vector<int> vec;

    // For every vertex in Hypercube, calculate its Hamming distance from startingVertex
    for (int i = 0; i < pow(2, HCDimension); i++) {
      // If the HammingdDistance is equal to the "dist" argument, insert it into a vector
      if (metrics::HammingdDistance(startingVertex, i) == dist){
        vec.push_back(i);
      }
    }

    // return the vector
    return vec;
  }

  /*
  This function returns a vector of vertices of size "probes".
  These are the probes kNN and RangeSearch will traverse.
  */
  std::vector<std::vector<Item<T>*>> _getAvailableProbes (int probes, int startingVertex) {
    // Initialize the vector
    std::vector<std::vector<Item<T>*>> probeVec;

    /*
    The maximum Hamming distance a vertex can have with another is HCDimension
    (essentially when all HCDimension bits are opposite).
    So starting from distance 0 (the startingVertex itself), get all probes of
    Hamming distance "i" until we have reached "probes" vertices
    */
    for (int i = 0; i < HCDimension; i++) {
      // Returns an array of indices (indices of H)
      std::vector<int> pph = _getProbesPerHD(i, startingVertex);

      // a is the amount of probes returned from _getProbesPerHD
      int a = 0;
      /*
      add vectors until we either reach "probes" amount of vertices
      or there are no more vertices with Hamming distance of i from startingVertex
      */
      for (int j = probeVec.size(); j < probes && a < pph.size(); j++, a++) {
        probeVec.push_back(H[pph[a]]);
      }

      // If we have reached "probes" amount of vertices, no need to continue loop
      if (probeVec.size() == probes){
        return probeVec;
      }
    }

    // If for some reason not all our vertices are enough, return every vertex
    return probeVec;
  }

public:
  Hypercube(interface::input::HC::HCInput& hci, const interface::Data<T>& ds, int ws, std::pair<int, int> F_Range)
  : windowSize(ws), fMin(F_Range.first), fMax(F_Range.second) {
    HCDimension = hci.k;
    imageCount = ds.n;
    dataDimension = ds.dimension;
    Item<T>** items = ds.items;

    // The amount of vertices the Hypercube contains
    int vertexAmount = pow(2, HCDimension);

    // H is an array of std::vector<Item<T>*> of size vertexAmount
    H = new std::vector<Item<T>*>[vertexAmount];
    for (int i = 0; i < vertexAmount; i++) {
      std::vector<Item<T>*> tmpVec;
      H[i] = tmpVec;
    }

    /*
    Calculate all m^b powers once to pass them to all HashFunction instances
    so as to not calculate them each time.
    */
    mConstant = pow(2, 32) - 5;
    m_mod_MValues = new int[dataDimension];
    for (int b = 0; b < dataDimension; b++)
      m_mod_MValues[b] = utils::modEx(mConstant, dataDimension-b-1, pow(2, 32/HCDimension));

    // Initialize HCDimension HashFunctions and FFunctions
    LSHFun = new HashFunction<T>*[HCDimension];
    mapFun = new FFunction*[HCDimension];
    for (int i = 0; i < HCDimension; i++) {
      LSHFun[i] = new HashFunction<T>(windowSize, HCDimension, dataDimension, m_mod_MValues);
      mapFun[i] = new FFunction(fMin, fMax);
    }

    // For every item given by the input set, add it to its respective vertex
    for (int i = 0; i < imageCount; i++) {
      int vertex = _calculateVertex(items[i]->data);
      H[vertex].push_back(items[i]);

      if ((i+1)%10000 == 0)
        std::cout << "Hypercube: " <<  i+1 << " training items..." << '\n';
    }
  }

  ~Hypercube () {
    for (int i = 0; i < HCDimension; i++) {
      delete LSHFun[i];
      delete mapFun[i];
    }
    delete[] LSHFun;
    delete[] mapFun;
    delete[] H;
    delete[] m_mod_MValues;
  }

  /*
  Each neighbor is represented as a pair of <distanceToQuery, neighborItem*>
  The following function returns a vector of these pairs
  */
  std::vector<std::pair<int, Item<T>*>> kNN (T* query, int N, int probes, int thresh = 0) {
    // At first initalize the vector itself
    std::vector<std::pair<int, Item<T>*>> d;
    // Then initialize each pair with distance -> (max integer) and a null item
    for (int i = 0; i < N; i++)
      d.push_back(std::make_pair(std::numeric_limits<int>::max(), new Item<T>()));

    // Calculate the startingVertex
    int vertex = _calculateVertex(query);
    // Fetch all vertices to be searched through _getAvailableProbes
    std::vector<std::vector<Item<T>*>> avProbes = _getAvailableProbes(probes, vertex);

    // Traverse all available probes until a threshold of itemsSearched is reached
    int itemsSearched = 0;
    for (int i = 0; i < avProbes.size(); i++) {
      for (int j = 0; j < avProbes[i].size(); j++) {
        // Calculate the distance from the current item to the query
        int distance = metrics::ManhattanDistance<T>(query, avProbes[i][j]->data, dataDimension);

        /*
        If the distance is less than the last pair's in the vector,
        replace the pair with the new distance and the current item.
        Then, sort the vector by ascending order based on distance.
        This is done so that whenever we find a good neighbor candidate,
        we replace the least similar neighbor in the vector
        */
        if (distance < d[N-1].first) {
          d[N-1].first = distance;
          if (d[N-1].second->null)
            delete d[N-1].second;
          d[N-1].second = avProbes[i][j];
          std::sort(d.begin(), d.end(), utils::comparePairs<T>);
        }

        // If threshold is 0, it indicates that there is no threshold
        if (thresh != 0 && ++itemsSearched >= thresh)
          return d;
      }
    }

    return d;
  }

  /*
  Each neighbor is represented as a pair of <distanceToQuery, neighborItem*>
  The following function returns a vector of these pairs
  */
  std::vector<std::pair<int, Item<T>*>> RangeSearch (T* query, double radius, int probes, int thresh = 0) {
    // Initialize the vector
    std::vector<std::pair<int, Item<T>*>> d;
    /*
    In this method, we do not need to sort the vector, also its size is not constant.
    Hence, we do not need to initalize its values.
    Simply, whenever a neighbor has distance less than radius, we add it to the vector
    */

    // Calculate the startingVertex
    int vertex = _calculateVertex(query);
    // Fetch all vertices to be searched through _getAvailableProbes
    std::vector<std::vector<Item<T>*>> avProbes = _getAvailableProbes(probes, vertex);

    // Traverse all available probes until a threshold of itemsSearched is reached
    int itemsSearched = 0;
    for (int i = 0; i < avProbes.size(); i++) {
      for (int j = 0; j < avProbes[i].size(); j++) {
        /*
        The following condition will only be met whenever this function is used by
        reverse assignment in clustering. When Hypercube is used for ANN, it will have no effect.
        In reverse assignment, to avoid fetching the same items, we "mark" them when inserted
        to a cluster so as to indicate that they are already assigned
        */
        if (avProbes[i][j]->marked)
          continue;

        // Calculate the distance from the current item to the query
        int distance = metrics::ManhattanDistance<T>(query, avProbes[i][j]->data, dataDimension);

        // If the distance is less than radius, insert the pair into the return vector
        if (distance < radius) {
          std::pair<int, Item<T>*> tmpPair = std::make_pair(distance, avProbes[i][j]);
          d.push_back(tmpPair);
        }

        if (thresh != 0 && ++itemsSearched >= thresh)
          return d;
      }
    }

    return d;
  }

  /*
  The following function creates a KNNOutput object.
  This object contains all information required to create the output file.
  This is used only on ANN (not clustering) and also requires that its brute force
  counterpart will be executed (so as to compare times and distances).
  */
  void buildOutput (interface::output::KNNOutput& output, interface::Dataset& query, int N, double R, int probes, int thresh = 0) {
    // A vector of all neighbors' ids returned by kNN for each query item
    std::vector<std::vector<int>> neighborIdVec;
    // The distances of the aforementioned neighbors to the query item
    std::vector<std::vector<double>> distVec;
    // The total time kNN took to calculate the k nearest neighbors
    std::vector<double> timeVec;
    // A vector of all neighbors' ids returned by RangeSearch for each query item
    std::vector<std::vector<int>> rsIdVec;

    // For each query item
    for (int i = 0; i < query.number_of_images; i++) {
      // Initialize temporary vectors to be added into neighborIdVec & distVec
      std::vector<int> tmpNVec;
      std::vector<double> tmpDistVec;

      // Execute kNN and calculate time elapsed
      clock_t begin = clock();

      std::vector<std::pair<int, Item<T>*>> kNNRes = kNN(query.images[i], N, probes, thresh);

      clock_t end = clock();
      double elapsed = double(end - begin) / CLOCKS_PER_SEC;

      // For every neighbor returned, add information to its respective vector
      for (int j = 0; j < kNNRes.size(); j++) {
        if (!kNNRes[j].second->null) {
          tmpNVec.push_back(kNNRes[j].second->id);
          tmpDistVec.push_back((double) kNNRes[j].first);
        }
      }

      // Initalize temporary vector to be added into rsIdVec
      std::vector<int> tmpRsVec;

      // Execute RangeSearch
      std::vector<std::pair<int, Item<T>*>> rsRes = RangeSearch(query.images[i], R, probes, thresh);

      // Add all neighbor ids into tmpRsVec
      for (int j = 0; j < rsRes.size(); j++) {
        if (!rsRes[j].second->null) {
          tmpRsVec.push_back(rsRes[j].second->id);
        }
      }

      if ((i+1)%1000 == 0)
        std::cout << "Hypercube: " << i+1 << " query items..." << '\n';

      // Push all temporary vectors into their "parent" vectors
      neighborIdVec.push_back(tmpNVec);
      distVec.push_back(tmpDistVec);
      timeVec.push_back(elapsed);
      rsIdVec.push_back(tmpRsVec);
    }

    // Set the following KNNOutput's attributes to the vectors created above
    output.n_neighbors_id = neighborIdVec;
    output.approx_distance = distVec;
    output.approx_time = timeVec;
    output.r_near_neighbors_id = rsIdVec;
  }
};
