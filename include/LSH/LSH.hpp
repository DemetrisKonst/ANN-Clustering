#include <vector>
#include <utility>
#include <algorithm>
#include <limits>

#include "LSHFun.hpp"
#include "../metrics/metrics.hpp"

template <typename T>
bool comparePairs (std::pair<int, T*> x, std::pair<int, T*> y) {
  return (x.first < y.first);
}

template <typename T>
class LSH {
private:
  int n;      //number of images
  int htSize;
  int D;      //dimension
  int k;      //number of hash functions for each amplified hash function
  int L;      //number of hash tables
  double r;      //search radius
  unsigned long int m;

  std::vector<T*>** H;
  AmplifiedHashFunction<T>** g;

public:
  LSH (int n, int div, int d, int k, int l, double r, unsigned long int m, T** data):
  n(n), D(d), k(k), L(l), r(r), m(m) {
    htSize = n/div;

    g = new AmplifiedHashFunction<T>*[L];
    H = new std::vector<T*>*[L];

    int* mmod = new int[D];
    for (int b = 0; b < D; b++)
      mmod[b] = utils::modEx(m, D-b-1, pow(2, 32/k));


    for (int i = 0; i < L; i++) {
      g[i] = new AmplifiedHashFunction<T>(r, 4, k, d, pow(2, 32) - 5, mmod);

      H[i] = new std::vector<T*>[htSize];
      for (int j = 0; j < htSize; j++) {
        std::vector<T*> tmpVec;
        H[i][j] = tmpVec;
      }
    }

    for (int a = 0; a < n; a++) {
      for (int i = 0; i < L; i++) {
        unsigned long int gres = g[i]->HashVector(data[a]);
        H[i][gres%htSize].push_back(data[a]);
      }

      if ((a+1)%10000 == 0)
        std::cout << a+1 << " items..." << '\n';
    }
  }

  std::vector<std::pair<int, T*>> ApproxNN (T* query, int N, int thresh) {
    std::vector<std::pair<int, T*>> d;

    for (int i = 0; i < N; i++)
      d.push_back(std::make_pair(std::numeric_limits<int>::max(), (T*) NULL));

    int itemsSearched = 0;
    for (int i = 0; i < L; i++) {
      int bucket = g[i]->HashVector(query)%htSize;

      for (int j = 0; j < H[i][bucket].size(); j++) {
        bool alreadyExists = false;
        for (int k = 0; k < N; k++)
          if (d[k].second == H[i][bucket][j])
            alreadyExists = true;

        if (alreadyExists)
          break;

        int distance = metrics::ManhattanDistance<T>(query, H[i][bucket][j], D);

        if (distance < d[N-1].first) {
          d[N-1].first = distance;
          d[N-1].second = H[i][bucket][j];
          std::sort(d.begin(), d.end(), comparePairs<T>);
        }

        if (++itemsSearched >= thresh)
          return d;
      }
    }

    return d;
  }

  std::vector<std::pair<int, T*>> RangeSearch (T* query, double radius, int thresh) {
    std::vector<std::pair<int, T*>> d;

    int itemsSearched = 0;
    for (int i = 0; i < L; i++) {
      int bucket = g[i]->HashVector(query)%htSize;

      for (int j = 0; j < H[i][bucket].size(); j++) {
        bool alreadyExists = false;
        for (int k = 0; k < d.size(); k++)
          if (d[k].second == H[i][bucket][j])
            alreadyExists = true;

        if (alreadyExists)
          break;

        int distance = metrics::ManhattanDistance<T>(query, H[i][bucket][j], D);

        if (distance < radius) {
          std::pair<int, T*> tmpPair = std::make_pair(distance, H[i][bucket][j]);
          d.push_back(tmpPair);
        }

        if (++itemsSearched >= thresh)
          return d;
      }
    }

    return d;
  }
};
