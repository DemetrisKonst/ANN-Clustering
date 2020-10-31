#include <vector>
#include <utility>
#include <algorithm>
#include <limits>
#include <ctime>

#include "../core/item.hpp"
#include "LSHFun.hpp"
#include "../metrics/metrics.hpp"
#include "../utils/ANN.hpp"

/*
The following class implements the LSH data structure.
It consists of L hash tables with their respective hash functions.
These hash functions are AmplifiedHashFunctions (implemented in LSHFun.hpp).
They map each item into a bucket for each hash table.
*/
template <typename T>
class LSH {
private:
  // Amount of images in training set
  int imageCount;
  // Data dimension
  int dimension;
  // Amount of hash functions for each amplified hash function
  int functionAmount;
  //Amount of hash tables(L)
  int htAmount;
  // Window size provided by the user
  int windowSize;
  // Size of hash tables
  int htSize;

  /*
  Each bucket is represented as a vector of items.
  Each hash table is an array containing "htSize" buckets.
  Each hash table exists inside an array of size "L"
  */
  std::vector<Item<T>*>** H;

  // Each hash table has its respective AmplifiedHashFunction
  AmplifiedHashFunction<T>** g;

  /*
  The following attributes are used in HashFunction.
  More details about them can be found there.
  */
  unsigned int mConstant;
  int* m_mod_MValues;

public:
  LSH (interface::input::LSH::LSHInput& lshi, const interface::Data<T>& ds, int ws) : windowSize(ws) {
    functionAmount = lshi.k;
    htAmount = lshi.L;
    imageCount = ds.n;
    dimension = ds.dimension;
    Item<T>** items = ds.items;

    /*
    div is the number with which we will divide the total number of images
    to calculate the size of each hash table. Proper values are 8 or 16
    */
    int div = 16;
    htSize = imageCount/div;

    // Calculate the m mod M values, these are used in HashFunction to calculate the return value
    mConstant = pow(2, 32) - 5;
    m_mod_MValues = new int[dimension];
    for (int b = 0; b < dimension; b++)
      m_mod_MValues[b] = utils::modEx(mConstant, dimension-b-1, pow(2, 32/functionAmount));

    // Initialize htAmount hash tables and AmplifiedHashFunctions
    H = new std::vector<Item<T>*>*[htAmount];
    g = new AmplifiedHashFunction<T>*[htAmount];

    for (int i = 0; i < htAmount; i++) {
      g[i] = new AmplifiedHashFunction<T>(windowSize, functionAmount, dimension, m_mod_MValues);

      H[i] = new std::vector<Item<T>*>[htSize];
      for (int j = 0; j < htSize; j++) {
        std::vector<Item<T>*> tmpVec;
        H[i][j] = tmpVec;
      }
    }

    // Hash all items in training set and insert them into their buckets
    for (int a = 0; a < imageCount; a++) {
      for (int i = 0; i < htAmount; i++) {
        unsigned int gres = g[i]->HashVector(items[a]->data);
        H[i][gres%htSize].push_back(items[a]);
      }

      if ((a+1)%10000 == 0)
        std::cout << "LSH: " << a+1 << " training items..." << '\n';
    }
  }

  ~LSH () {
    for (int i = 0; i < htAmount; i++) {
      delete[] H[i];
      delete g[i];
    }
    delete[] H;
    delete[] g;
    delete[] m_mod_MValues;
  }

  /*
  Each neighbor is represented as a pair of <distanceToQuery, neighborItem*>
  The following function returns a vector of these pairs
  */
  std::vector<std::pair<int, Item<T>*>> kNN (T* query, int N, int thresh = 0) {
    // At first initalize the vector itself
    std::vector<std::pair<int, Item<T>*>> d;
    // Then initialize each pair with distance -> (max integer) and a null item
    for (int i = 0; i < N; i++)
      d.push_back(std::make_pair(std::numeric_limits<int>::max(), new Item<T>()));

    // For each hash table...
    int itemsSearched = 0;
    for (int i = 0; i < htAmount; i++) {
      // Calculate the bucket to which the query item corresponds
      int bucket = g[i]->HashVector(query)%htSize;

      // For each item inside the bucket...
      for (int j = 0; j < H[i][bucket].size(); j++) {
        /*
        Check if the current item is already inserted into the vector
        (it is possible since it may have been inserted from a previous hash table loop)
        */
        bool alreadyExists = false;
        for (int a = 0; a < N; a++)
          if (d[a].second->id == H[i][bucket][j]->id)
            alreadyExists = true;

        if (alreadyExists)
          continue;

        // Since it does not exist in the vector, calculate its distance to the query item
        int distance = metrics::ManhattanDistance<T>(query, H[i][bucket][j]->data, dimension);

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
          d[N-1].second = H[i][bucket][j];
          std::sort(d.begin(), d.end(), utils::comparePairs<T>);
        }

        /*
        If a certain threshold of items traversed is reached, return the vector.
        If thresh == 0 it indicates that the user does not want to add a threshold.
        */
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
  std::vector<std::pair<int, Item<T>*>> RangeSearch (T* query, double radius, int thresh = 0) {
    // Initialize the vector
    std::vector<std::pair<int, Item<T>*>> d;
    /*
    In this method, we do not need to sort the vector, also its size is not constant.
    Hence, we do not need to initalize its values.
    Simply, whenever a neighbor has distance less than radius, we add it to the vector
    */

    // For each hash table...
    int itemsSearched = 0;
    for (int i = 0; i < htAmount; i++) {
      // Calculate the bucket to which the query item corresponds
      int bucket = g[i]->HashVector(query)%htSize;

      // For each item inside the bucket...
      for (int j = 0; j < H[i][bucket].size(); j++) {
        // Check if the current item is already inserted into the vector
        bool alreadyExists = false;
        for (int a = 0; a < d.size(); a++)
          if (d[a].second->id == H[i][bucket][j]->id)
            alreadyExists = true;

        /*
        The "marked" condition will only be met whenever this function is used by
        reverse assignment in clustering. When LSH is used for ANN, it will have no effect.
        In reverse assignment, to avoid fetching the same items, we "mark" them when inserted
        to a cluster so as to indicate that they are already assigned.
        */
        if (alreadyExists || H[i][bucket][j]->marked)
          continue;

        int distance = metrics::ManhattanDistance<T>(query, H[i][bucket][j]->data, dimension);

        // If the distance is less than radius, insert the pair into the return vector
        if (distance < radius) {
          std::pair<int, Item<T>*> tmpPair = std::make_pair(distance, H[i][bucket][j]);
          d.push_back(tmpPair);
        }

        // If a certain threshold of items traversed is reached, return the vector.
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
  void buildOutput (interface::output::KNNOutput& output, interface::Dataset& query, int N, double R, int thresh = 0) {
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

      std::vector<std::pair<int, Item<T>*>> kNNRes = kNN(query.images[i], N, thresh);

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
      std::vector<std::pair<int, Item<T>*>> rsRes = RangeSearch(query.images[i], R, thresh);

      // Add all neighbor ids into tmpRsVec
      for (int j = 0; j < rsRes.size(); j++) {
        if (!rsRes[j].second->null) {
          tmpRsVec.push_back(rsRes[j].second->id);
        }
      }

      if ((i+1)%1000 == 0)
        std::cout << "LSH: " << i+1 << " query items..." << '\n';

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
