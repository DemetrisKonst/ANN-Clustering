#include <vector>
#include <utility>
#include <algorithm>
#include <limits>

#include "../core/item.hpp"
#include "LSHFun.hpp"
#include "../metrics/metrics.hpp"

template <typename T>
bool comparePairs (std::pair<int, Item<T>*> x, std::pair<int, Item<T>*> y) {
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

  std::vector<Item<T>*>** H;
  AmplifiedHashFunction<T>** g;

public:
  LSH (int n, int div, int d, int k, int l, double r, unsigned long int m, Item<T>** items):
  n(n), D(d), k(k), L(l), r(r), m(m) {
    htSize = n/div;

    g = new AmplifiedHashFunction<T>*[L];
    H = new std::vector<Item<T>*>*[L];

    int* mmod = new int[D];
    for (int b = 0; b < D; b++)
      mmod[b] = utils::modEx(m, D-b-1, pow(2, 32/k));


    for (int i = 0; i < L; i++) {
      g[i] = new AmplifiedHashFunction<T>(r, 4, k, d, pow(2, 32) - 5, mmod);

      H[i] = new std::vector<Item<T>*>[htSize];
      for (int j = 0; j < htSize; j++) {
        std::vector<Item<T>> tmpVec;
        H[i][j] = tmpVec;
      }
    }

    for (int a = 0; a < n; a++) {
      for (int i = 0; i < L; i++) {
        unsigned long int gres = g[i]->HashVector(items[a]->data);
        H[i][gres%htSize].push_back(items[a]);
      }

      if ((a+1)%10000 == 0)
        std::cout << a+1 << " items..." << '\n';
    }
  }

  std::vector<std::pair<int, Item<T>*>> ApproxNN (T* query, int N, int thresh = 0) {
    std::vector<std::pair<int, Item<T>*>> d;

    for (int i = 0; i < N; i++)
      d.push_back(std::make_pair(std::numeric_limits<int>::max(), new Item<T>()));

    int itemsSearched = 0;
    for (int i = 0; i < L; i++) {
      int bucket = g[i]->HashVector(query)%htSize;

      for (int j = 0; j < H[i][bucket].size(); j++) {
        bool alreadyExists = false;
        for (int k = 0; k < N; k++)
          if (d[k].second->id == H[i][bucket][j]->id)
            alreadyExists = true;

        if (alreadyExists)
          continue;

        int distance = metrics::ManhattanDistance<T>(query, H[i][bucket][j]->data, D);

        if (distance < d[N-1].first) {
          d[N-1].first = distance;
          d[N-1].second = H[i][bucket][j];
          std::sort(d.begin(), d.end(), comparePairs<T>);
        }

        if (thresh != 0 && ++itemsSearched >= thresh)
          return d;
      }
    }

    return d;
  }

  std::vector<std::pair<int, Item<T>*>> RangeSearch (T* query, double radius, int thresh = 0) {
    std::vector<std::pair<int, Item<T>*>> d;

    int itemsSearched = 0;
    for (int i = 0; i < L; i++) {
      int bucket = g[i]->HashVector(query)%htSize;

      for (int j = 0; j < H[i][bucket].size(); j++) {
        bool alreadyExists = false;
        for (int k = 0; k < d.size(); k++)
          if (d[k].second->id == H[i][bucket][j]->id)
            alreadyExists = true;

        if (alreadyExists || H[i][bucket][j]->marked)
          continue;

        int distance = metrics::ManhattanDistance<T>(query, H[i][bucket][j]->data, D);

        if (distance < radius) {
          std::pair<int, Item<T>*> tmpPair = std::make_pair(distance, H[i][bucket][j]);
          d.push_back(tmpPair);
        }

        if (thresh != 0 && ++itemsSearched >= thresh)
          return d;
      }
    }

    return d;
  }
};
