#include <unordered_map>

#include "../core/item.hpp"
#include "../LSH/LSHFun.hpp"
#include "../metrics/metrics.hpp"

template <typename T>
bool comparePairs (std::pair<int, Item<T>> x, std::pair<int, Item<T>> y) {
  return (x.first < y.first);
}

template <typename T>
class Hypercube {
private:
  int n;
  int HCdim;
  int datadim;
  double searchRadius;
  int m;

  std::vector<HashFunction<T>> LSHFun;

  std::vector<Item<T>>* H;
public:
  Hypercube(int n, int hcd, int dd, double sr, unsigned long int m, T** data):
  n(n), HCdim(hcd), datadim(dd), searchRadius(sr), m(m){
    int vertexCount = pow(2, HCdim);

    H = new std::vector<Item<T>>[vertexCount];

    for (int i = 0; i < vertexCount; i++) {
      std::vector<Item<T>> tmpVec;
      H[i] = tmpVec;
    }

    for (int i = 0; i < HCdim; i++) {
      LSHFun.push_back(HashFunction<T>(searchRadius, 4, 4, datadim, m));
    }

    for (int i = 0; i < n; i++) {
      int vertex = calculateVertex(data[i]);
      Item<T> tmpItem(i, datadim, data[i]);
      H[vertex].push_back(tmpItem);

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

  std::vector<std::vector<Item<T>>> getAvailableProbes (int probes, int startingVertex) {
    std::vector<std::vector<Item<T>>> probeVec;

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

  std::vector<std::pair<int, Item<T>>> ApproxNN (T* query, int N, int probes, int thresh) {
    std::vector<std::pair<int, Item<T>>> d;

    for (int i = 0; i < N; i++)
      d.push_back(std::make_pair(std::numeric_limits<int>::max(), Item<T>()));

    int vertex = calculateVertex(query);
    std::vector<std::vector<Item<T>>> avProbes = getAvailableProbes(probes, vertex);

    int itemsSearched = 0;
    for (int i = 0; i < avProbes.size(); i++) {
      for (int j = 0; j < avProbes[i].size(); j++) {
        int distance = metrics::ManhattanDistance<T>(query, avProbes[i][j].data, datadim);

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

  std::vector<std::pair<int, Item<T>>> RangeSearch (T* query, double radius, int probes, int thresh) {
    std::vector<std::pair<int, Item<T>>> d;

    int vertex = calculateVertex(query);
    std::vector<std::vector<Item<T>>> avProbes = getAvailableProbes(probes, vertex);

    int itemsSearched = 0;
    for (int i = 0; i < avProbes.size(); i++) {
      for (int j = 0; j < avProbes[i].size(); j++) {
        int distance = metrics::ManhattanDistance<T>(query, avProbes[i][j].data, datadim);

        if (distance < radius) {
          std::pair<int, Item<T>> tmpPair = std::make_pair(distance, avProbes[i][j]);
          d.push_back(tmpPair);
        }

        if (++itemsSearched >= thresh)
          return d;
      }
    }

    return d;
  }
};