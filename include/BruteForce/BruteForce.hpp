#include <utility>
#include <limits>
#include <ctime>
#include <cmath>

#include "../core/item.hpp"
#include "../metrics/metrics.hpp"
#include "../utils/lsh_hc.hpp"

template <typename T>
class BruteForce {
private:
  int imageCount;
  int dimension;
  Item<T>** items;

public:
  BruteForce (interface::Data<T>& ds) {
    imageCount = ds.n;
    dimension = ds.dimension;
    items = ds.items;
  }

  double averageDistance (double datasetPercentage) {
    int partitionSize = floor(datasetPercentage*imageCount);

    std::cout << partitionSize << " out of " << imageCount << '\n';

    double outerSum = 0.0;
    for (int i = 0; i < partitionSize; i++) {
      int innerSum = 0;
      for (int j = 0; j < partitionSize; j++) {
        if (i == j) continue;

        innerSum += metrics::ManhattanDistance<T>(items[i]->data, items[j]->data, dimension);
      }
      outerSum += 1.0*innerSum/partitionSize;
    }

    return outerSum/partitionSize;
  }

  std::vector<std::pair<int, Item<T>*>> kNN (T* query, int N, int thresh = 0) {
    std::vector<std::pair<int, Item<T>*>> d;

    for (int i = 0; i < N; i++)
      d.push_back(std::make_pair(std::numeric_limits<int>::max(), new Item<T>()));


    for (int i = 0; i < imageCount; i++) {
      int distance = metrics::ManhattanDistance<T>(query, items[i]->data, dimension);

      if (distance < d[N-1].first) {
        d[N-1].first = distance;
        d[N-1].second = items[i];
        std::sort(d.begin(), d.end(), comparePairs<T>);
      }

      if (thresh != 0 && i >= thresh)
        return d;
    }

    return d;
  }

  std::pair<int, T*> RangeSearch (T* query, double radius) {
    for (int i = 0; i < imageCount; i++) {
      int distance = metrics::ManhattanDistance<T>(items[i]->data, query, dimension);

      if (distance < radius) {
        return std::make_pair(distance, items[i]);
      }
    }
  }

  void buildOutput (interface::output::KNNOutput& output, interface::Dataset& query, int N, int thresh = 0) {
    std::vector<std::vector<double>> distVec;
    std::vector<double> timeVec;
    std::vector<int> queryIdVec;

    for (int i = 0; i < query.number_of_images; i++) {
      std::vector<double> tmpDistVec;
      clock_t begin = clock();

      std::vector<std::pair<int, Item<T>*>> kNNRes = kNN(query.images[i], N, thresh);

      clock_t end = clock();
      double elapsed = double(end - begin) / CLOCKS_PER_SEC;

      for (int j = 0; j < kNNRes.size(); j++){
        if (!kNNRes[j].second->null){
          tmpDistVec.push_back( (double) kNNRes[j].first);
        }
      }

      distVec.push_back(tmpDistVec);
      timeVec.push_back(elapsed);
      queryIdVec.push_back(i);
    }

    output.true_distance = distVec;
    output.true_time = timeVec;
    output.query_id = queryIdVec;
  }
};
