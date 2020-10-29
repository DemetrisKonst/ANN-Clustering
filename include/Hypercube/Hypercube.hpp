#include <random>

#include "../core/item.hpp"
#include "../LSH/LSHFun.hpp"
#include "../metrics/metrics.hpp"
#include "../interfaces/HC_interface.h"
#include "../utils/lsh_hc.hpp"

class FFunc {
private:
  unsigned int randomNumber;

public:
  FFunc () {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(100, 200);

    randomNumber = distribution(generator);
  }

  int map (unsigned int hVal) {
    return hVal >= randomNumber;
  }
};

template <typename T>
class Hypercube {
private:
  int imageCount;
  int HCDimension;
  int dataDimension;
  double averageItemDistance;

  std::vector<FFunc> mapFun;
  std::vector<HashFunction<T>> LSHFun;

  std::vector<Item<T>*>* H;

  // The following function maps the result of H to {0, 1}

  int _calculateVertex (T* x) {
    int vertex = 0;

    for (int i = 0; i < HCDimension; i++) {
      int hashRes = LSHFun[i].HashVector(x);
      // std::cout << hashRes << "->";
      int mapRes = mapFun[i].map(hashRes);

      std::cout << mapRes;

      if (mapRes == 1) {
        vertex |= 1;
      }

      if (i < HCDimension-1)
        vertex <<= 1;
    }

    std::cout << '\n';

    return vertex;
  }

  /*
  The following function returns all vertex indices of distance "dist" from startingVertex
  */
  std::vector<int> _getProbesPerHD (int dist, int startingVertex) {
    std::vector<int> vec;

    for (int i = 0; i < pow(2, HCDimension); i++) {
      if (metrics::HammingdDistance(startingVertex, i) == dist){
        vec.push_back(i);
      }
    }

    return vec;
  }

  /*
  This function returns an array of "probes" amount of the closest vertices to startingVertex
  */
  std::vector<std::vector<Item<T>*>> _getAvailableProbes (int probes, int startingVertex) {
    std::vector<std::vector<Item<T>*>> probeVec;

    for (int i = 0; i < HCDimension; i++) {
      std::vector<int> pph = _getProbesPerHD(i, startingVertex);

      int a = 0;
      for (int j = probeVec.size(); j < probes && a < pph.size(); j++, a++) {
        probeVec.push_back(H[pph[a]]);
      }

      if (probeVec.size() == probes){
        return probeVec;
      }
    }

    return probeVec;
  }

public:
  Hypercube(interface::input::HC::HCInput& hci, const interface::Data<T>& ds, double avg) : averageItemDistance(avg) {
    HCDimension = hci.k;
    imageCount = ds.n;
    dataDimension = ds.dimension;
    Item<T>** items = ds.items;

    int vertexCount = pow(2, HCDimension);

    H = new std::vector<Item<T>*>[vertexCount];
    for (int i = 0; i < vertexCount; i++) {
      std::vector<Item<T>*> tmpVec;
      H[i] = tmpVec;
    }

    long int mConstant = pow(2, 32) - 5;
    int* mmod = new int[dataDimension];
    for (int b = 0; b < dataDimension; b++)
      mmod[b] = utils::modEx(mConstant, dataDimension-b-1, pow(2, 32/HCDimension));

    for (int i = 0; i < HCDimension; i++) {
      LSHFun.push_back(HashFunction<T>(averageItemDistance, 4, 4, dataDimension, mmod));
      mapFun.push_back(FFunc());
    }

    for (int i = 0; i < imageCount; i++) {
      int vertex = _calculateVertex(items[i]->data);
      H[vertex].push_back(items[i]);

      if ((i+1)%10000 == 0)
        std::cout << i+1 << " items..." << '\n';
    }
  }

  std::vector<std::pair<int, Item<T>*>> kNN (T* query, int N, int probes, int thresh = 0) {
    std::vector<std::pair<int, Item<T>*>> d;

    for (int i = 0; i < N; i++)
      d.push_back(std::make_pair(std::numeric_limits<int>::max(), new Item<T>()));

    int vertex = _calculateVertex(query);
    std::vector<std::vector<Item<T>*>> avProbes = _getAvailableProbes(probes, vertex);

    int itemsSearched = 0;
    for (int i = 0; i < avProbes.size(); i++) {
      for (int j = 0; j < avProbes[i].size(); j++) {
        int distance = metrics::ManhattanDistance<T>(query, avProbes[i][j]->data, dataDimension);

        if (distance < d[N-1].first) {
          d[N-1].first = distance;
          d[N-1].second = avProbes[i][j];
          std::sort(d.begin(), d.end(), comparePairs<T>);
        }

        if (thresh != 0 && ++itemsSearched >= thresh)
          return d;
      }
    }

    return d;
  }

  std::vector<std::pair<int, Item<T>*>> RangeSearch (T* query, double radius, int probes, int thresh) {
    std::vector<std::pair<int, Item<T>*>> d;

    int vertex = _calculateVertex(query);
    std::vector<std::vector<Item<T>*>> avProbes = _getAvailableProbes(probes, vertex);

    int itemsSearched = 0;
    for (int i = 0; i < avProbes.size(); i++) {
      for (int j = 0; j < avProbes[i].size(); j++) {
        if (avProbes[i][j]->marked)
          continue;

        int distance = metrics::ManhattanDistance<T>(query, avProbes[i][j]->data, dataDimension);

        if (distance < radius) {
          std::pair<int, Item<T>*> tmpPair = std::make_pair(distance, avProbes[i][j]);
          d.push_back(tmpPair);
        }

        if (++itemsSearched >= thresh)
          return d;
      }
    }

    return d;
  }

  void buildOutput (interface::output::KNNOutput& output, interface::Dataset& query, int N, double R, int probes, int thresh = 0) {
    std::vector<std::vector<int>> neighborIdVec;
    std::vector<std::vector<double>> distVec;
    std::vector<double> timeVec;
    std::vector<std::vector<int>> rsIdVec;

    for (int i = 0; i < 5; i++) {
      std::vector<int> tmpNVec;
      std::vector<double> tmpDistVec;

      clock_t begin = clock();

      std::vector<std::pair<int, Item<T>*>> kNNRes = kNN(query.images[i], N, probes, thresh);

      clock_t end = clock();
      double elapsed = double(end - begin) / CLOCKS_PER_SEC;

      for (int j = 0; j < kNNRes.size(); j++) {
        if (!kNNRes[j].second->null) {
          tmpNVec.push_back(kNNRes[j].second->id);
          tmpDistVec.push_back((double) kNNRes[j].first);
        }
      }

      std::vector<int> tmpRsVec;

      std::vector<std::pair<int, Item<T>*>> rsRes = RangeSearch(query.images[i], R, probes, thresh);

      for (int j = 0; j < rsRes.size(); j++) {
        if (!rsRes[j].second->null) {
          tmpRsVec.push_back(rsRes[j].second->id);
        }
      }

      neighborIdVec.push_back(tmpNVec);
      distVec.push_back(tmpDistVec);
      timeVec.push_back(elapsed);
      rsIdVec.push_back(tmpRsVec);
    }

    output.n_neighbors_id = neighborIdVec;
    output.approx_distance = distVec;
    output.approx_time = timeVec;
    output.r_near_neighbors_id = rsIdVec;
  }
};
