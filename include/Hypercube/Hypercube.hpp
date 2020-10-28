#include <unordered_map>

#include "../core/item.hpp"
#include "../LSH/LSHFun.hpp"
#include "../metrics/metrics.hpp"
#include "../interfaces/HC_interface.h"
#include "../utils/lsh_hc.hpp"

template <typename T>
class Hypercube {
private:
  int n;
  int HCdim;
  int datadim;
  double searchRadius;
  int m;

  std::vector<HashFunction<T>> LSHFun;

  std::vector<Item<T>*>* H;
public:
  Hypercube(interface::input::HC::HCInput& hci, interface::Data<T>& ds) {
    HCdim = hci.k;
    searchRadius = hci.R;
    n = ds.n;
    datadim = ds.dimension;
    Item<T>** items = ds.items;
    m = pow(2, 32) - 5;

    int vertexCount = pow(2, HCdim);

    H = new std::vector<Item<T>*>[vertexCount];

    for (int i = 0; i < vertexCount; i++) {
      std::vector<Item<T>*> tmpVec;
      H[i] = tmpVec;
    }

    for (int i = 0; i < HCdim; i++) {
      LSHFun.push_back(HashFunction<T>(searchRadius, 4, 4, datadim, m));
    }

    for (int i = 0; i < n; i++) {
      int vertex = calculateVertex(items[i]->data);
      H[vertex].push_back(items[i]);

      if ((i+1)%10000 == 0)
        std::cout << i+1 << " items..." << '\n';
    }
  }

  int calculateVertex (T* x) {
    int vertex = 0;
    for (int i = 0; i < HCdim; i++) {
      T hashRes = LSHFun[i].HashVector(x);

      if (hashRes%2 == 1) {
        vertex |= 1;
      }

      if (i < HCdim-1)
        vertex <<= 1;

    }

    return vertex;
  }

  std::vector<int> getProbesPerHD (int dist, int startingVertex) {
    std::vector<int> vec;

    for (int i = 0; i < pow(2, HCdim); i++) {
      if (metrics::HammingdDistance(startingVertex, i) == dist){
        vec.push_back(i);
      }
    }

    return vec;
  }

  std::vector<std::vector<Item<T>*>> getAvailableProbes (int probes, int startingVertex) {
    std::vector<std::vector<Item<T>*>> probeVec;

    for (int i = 0; i < HCdim; i++) {
      std::vector<int> pph = getProbesPerHD(i, startingVertex);

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

  std::vector<std::pair<int, Item<T>*>> kNN (T* query, int N, int probes, int thresh) {
    std::vector<std::pair<int, Item<T>*>> d;

    for (int i = 0; i < N; i++)
      d.push_back(std::make_pair(std::numeric_limits<int>::max(), new Item<T>()));

    int vertex = calculateVertex(query);
    std::vector<std::vector<Item<T>*>> avProbes = getAvailableProbes(probes, vertex);

    int itemsSearched = 0;
    for (int i = 0; i < avProbes.size(); i++) {
      for (int j = 0; j < avProbes[i].size(); j++) {
        int distance = metrics::ManhattanDistance<T>(query, avProbes[i][j]->data, datadim);

        if (distance < d[N-1].first) {
          d[N-1].first = distance;
          d[N-1].second = avProbes[i][j];
          std::sort(d.begin(), d.end(), comparePairs<T>);
        }

        if (++itemsSearched >= thresh)
          return d;
      }
    }

    return d;
  }

  std::vector<std::pair<int, Item<T>*>> RangeSearch (T* query, double radius, int probes, int thresh) {
    std::vector<std::pair<int, Item<T>*>> d;

    int vertex = calculateVertex(query);
    std::vector<std::vector<Item<T>*>> avProbes = getAvailableProbes(probes, vertex);

    int itemsSearched = 0;
    for (int i = 0; i < avProbes.size(); i++) {
      for (int j = 0; j < avProbes[i].size(); j++) {
        if (avProbes[i][j]->marked)
          continue;

        int distance = metrics::ManhattanDistance<T>(query, avProbes[i][j]->data, datadim);

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

  std::vector<std::vector<std::pair<int, Item<T>*>>> buildOutput (T** query, int N, int probes, int thresh = 0) {
    std::vector<std::vector<std::pair<int, Item<T>*>>> hcVec;

    for (int i = 0; i < query.number_of_images; i++) {
      hcVec.push_back(kNN(query[i], N, probes, thresh));
    }

    return hcVec;
  }
};
