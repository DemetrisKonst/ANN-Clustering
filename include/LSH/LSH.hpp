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
  int imageCount;      //number of images
  int htSize;
  int dimension;      //dimension
  int functionAmount;      //number of hash functions for each amplified hash function
  int htAmount;      //number of hash tables
  double searchRadius;      //search radius
  unsigned long int mConstant;

  std::vector<Item<T>*>** H;
  AmplifiedHashFunction<T>** g;

public:
  LSH (interface::input::LSH::LSHInput lshi, const interface::Data<T>& ds) {
    functionAmount = lshi.k;
    htAmount = lshi.L;
    searchRadius = lshi.R;
    imageCount = ds.n;
    dimension = ds.dimension;
    Item<T>** items = ds.items;
    mConstant = pow(2, 32) - 5;
    int div = 16;

    htSize = imageCount/div;

    g = new AmplifiedHashFunction<T>*[htAmount];
    H = new std::vector<Item<T>*>*[htAmount];

    int* mmod = new int[dimension];
    for (int b = 0; b < dimension; b++)
      mmod[b] = utils::modEx(mConstant, dimension-b-1, pow(2, 32/functionAmount));


    for (int i = 0; i < htAmount; i++) {
      g[i] = new AmplifiedHashFunction<T>(searchRadius, 4, functionAmount, dimension, mmod);

      H[i] = new std::vector<Item<T>*>[htSize];
      for (int j = 0; j < htSize; j++) {
        std::vector<Item<T>*> tmpVec;
        H[i][j] = tmpVec;
      }
    }

    for (int a = 0; a < imageCount; a++) {
      for (int i = 0; i < htAmount; i++) {
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
    for (int i = 0; i < htAmount; i++) {
      int bucket = g[i]->HashVector(query)%htSize;

      for (int j = 0; j < H[i][bucket].size(); j++) {
        bool alreadyExists = false;
        for (int a = 0; a < N; a++)
          if (d[a].second->id == H[i][bucket][j]->id)
            alreadyExists = true;

        if (alreadyExists)
          continue;

        int distance = metrics::ManhattanDistance<T>(query, H[i][bucket][j]->data, dimension);

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
    for (int i = 0; i < htAmount; i++) {
      int bucket = g[i]->HashVector(query)%htSize;

      for (int j = 0; j < H[i][bucket].size(); j++) {
        bool alreadyExists = false;
        for (int a = 0; a < d.size(); a++)
          if (d[a].second->id == H[i][bucket][j]->id)
            alreadyExists = true;

        if (alreadyExists || H[i][bucket][j]->marked)
          continue;

        int distance = metrics::ManhattanDistance<T>(query, H[i][bucket][j]->data, dimension);

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

  void buildOutput (interface::output::KNNOutput& output, const interface::Data<T>& queryData) {
    output.n = queryData.n;
    output.R = 0;
    output.method = "LSH";

  }
};
