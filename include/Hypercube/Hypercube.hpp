#include <unordered_map>

#include "../LSH/LSHFun.hpp"
#include "../metrics/metrics.hpp"

bool comparePairs (std::pair<int, uint8_t*> x, std::pair<int, uint8_t*> y) {
  return (x.first < y.first);
}

class Hypercube {
private:
  int n;
  int HCdim;
  int datadim;
  double searchRadius;
  int m;

  std::vector<HashFunction> LSHFun;

  std::vector<uint8_t*>* H;
public:
  Hypercube(int n, int hcd, int dd, double sr, unsigned long int m, uint8_t** data):
  n(n), HCdim(hcd), datadim(dd), searchRadius(sr), m(m){
    int vertexCount = pow(2, HCdim);

    std::cout << vertexCount << '\n';

    H = new std::vector<uint8_t*>[vertexCount];

    for (int i = 0; i < vertexCount; i++) {
      std::vector<uint8_t*> tmpVec;
      H[i] = tmpVec;
    }

    for (int i = 0; i < HCdim; i++) {
      LSHFun.push_back(HashFunction(searchRadius, 4, 4, datadim, m));
    }

    for (int i = 0; i < n; i++) {
      int vertex = calculateVertex(data[i]);
      H[vertex].push_back(data[i]);

      if (i%10000 == 0)
        std::cout << "Calculated " << i << " records..." << '\n';
    }
  }

  int calculateVertex (uint8_t* x) {
    int vertex = 0;
    for (int i = 0; i < HCdim; i++) {
      uint8_t hashRes = LSHFun[i].HashVector(x);

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

  std::vector<std::vector<uint8_t*>> getAvailableProbes (int probes, int startingVertex) {
    std::vector<std::vector<uint8_t*>> probeVec;

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

  std::vector<std::pair<int, uint8_t*>> ApproxNN (uint8_t* query, int N, int probes, int thresh) {
    std::vector<std::pair<int, uint8_t*>> d;

    for (int i = 0; i < N; i++)
      d.push_back(std::make_pair(std::numeric_limits<int>::max(), (uint8_t*) NULL));

    int vertex = calculateVertex(query);
    std::vector<std::vector<uint8_t*>> avProbes = getAvailableProbes(probes, vertex);

    int imagesSearched = 0;
    for (int i = 0; i < avProbes.size(); i++) {
      for (int j = 0; j < avProbes[i].size(); j++) {
        int distance = metrics::ManhattanDistance(query, avProbes[i][j]);

        if (distance < d[N-1].first) {
          d[N-1].first = distance;
          d[N-1].second = avProbes[i][j];
          std::sort(d.begin(), d.end(), comparePairs);
        }

        if (++imagesSearched == thresh)
          return d;
      }
    }

    std::cout << "Images Searched: " << imagesSearched << '\n';
    return d;
  }

  std::vector<std::pair<int, uint8_t*>> RangeSearch (uint8_t* query, double radius, int probes, int thresh) {
    std::vector<std::pair<int, uint8_t*>> d;

    int vertex = calculateVertex(query);
    std::vector<std::vector<uint8_t*>> avProbes = getAvailableProbes(probes, vertex);

    int imagesSearched = 0;

    for (int i = 0; i < avProbes.size(); i++) {
      for (int j = 0; j < avProbes[i].size(); j++) {
        int distance = metrics::ManhattanDistance(query, avProbes[i][j]);

        if (distance < radius) {
          std::pair<int, uint8_t*> tmpPair = std::make_pair(distance, avProbes[i][j]);
          d.push_back(tmpPair);
        }

        if (++imagesSearched == thresh)
          return d;
      }
    }

    std::cout << "Images Searched: " << imagesSearched << '\n';
    return d;
  }
};
