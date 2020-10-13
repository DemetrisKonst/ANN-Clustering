#include <vector>
#include <utility>
#include <algorithm>
#include <limits>

#include "LSHFun.hpp"
#include "../metrics/metrics.hpp"

bool comparePairs (std::pair<int, uint8_t*> x, std::pair<int, uint8_t*> y) {
  return (x.first < y.first);
}

class LSH {
private:
  int n;      //number of images
  int htSize;
  int D;      //dimension
  int k;      //number of hash functions for each amplified hash function
  int L;      //number of hash tables
  double r;      //search radius
  unsigned long int m;

  std::vector<uint8_t*>** H;
  AmplifiedHashFunction** g;

public:
  LSH (int n, int div, int d, int k, int l, double r, unsigned long int m, uint8_t** data):
  n(n), D(d), k(k), L(l), r(r), m(m) {
    htSize = n/div;
    g = new AmplifiedHashFunction*[L];
    H = new std::vector<uint8_t*>*[L];

    int* mmod = new int[D];
    for (int b = 0; b < D; b++)
      mmod[b] = utils::modEx(m, D-b-1, pow(2, 32/k));

    for (int i = 0; i < L; i++) {
      g[i] = new AmplifiedHashFunction(r, 4, k, d, pow(2, 32) - 5, mmod);

      H[i] = new std::vector<uint8_t*>[htSize];
      for (int j = 0; j < htSize; j++) {
        std::vector<uint8_t*> tmpVec;
        H[i][j] = tmpVec;
      }
    }

    for (int a = 0; a < n; a++) {
      for (int i = 0; i < L; i++) {
        unsigned long int gres = g[i]->HashVector(data[a]);
        H[i][gres%htSize].push_back(data[a]);
      }
    }
  }

  std::vector<std::pair<int, uint8_t*>> ApproxNN (uint8_t* query, int N) {
    std::vector<std::pair<int, uint8_t*>> d;

    for (int i = 0; i < N; i++)
      d.push_back(std::make_pair(std::numeric_limits<int>::max(), (uint8_t*) NULL));

    for (int i = 0; i < L; i++) {
      int bucket = g[i]->HashVector(query)%htSize;

      for (int j = 0; j < H[i][bucket].size(); j++) {
        bool alreadyExists = false;
        for (int k = 0; k < N; k++)
          if (d[k].second == H[i][bucket][j])
            alreadyExists = true;

        if (alreadyExists)
          break;

        int distance = metrics::ManhattanDistance(query, H[i][bucket][j]);

        // std::cout << distance << " < " << d[N-1].first << "\n";
        if (distance < d[N-1].first) {
          d[N-1].first = distance;
          d[N-1].second = H[i][bucket][j];
          std::sort(d.begin(), d.end(), comparePairs);
        }
      }
    }

    return d;
  }

  std::vector<std::pair<int, uint8_t*>> RangeSearch (uint8_t* query, double radius) {
    std::vector<std::pair<int, uint8_t*>> d;

    for (int i = 0; i < L; i++) {
      int bucket = g[i]->HashVector(query)%htSize;

      for (int j = 0; j < H[i][bucket].size(); j++) {
        bool alreadyExists = false;
        for (int k = 0; k < d.size(); k++)
          if (d[k].second == H[i][bucket][j])
            alreadyExists = true;

        if (alreadyExists)
          break;

        int distance = metrics::ManhattanDistance(query, H[i][bucket][j]);

        if (distance < radius) {
          std::pair<int, uint8_t*> tmpPair = std::make_pair(distance, H[i][bucket][j]);
          d.push_back(tmpPair);
        }
      }
    }

    return d;
  }
};
