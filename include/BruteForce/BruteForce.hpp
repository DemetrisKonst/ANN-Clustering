#include <utility>
#include <limits>
#include <ctime>
#include <cmath>

#include "../core/item.hpp"
#include "../metrics/metrics.hpp"
#include "../utils/lsh_hc.hpp"

/*
The following class implements a simple brute force implementation of kNN and RangeSearch
*/
template <typename T>
class BruteForce {
private:
  // Number of items in training set
  int imageCount;
  // Data dimension
  int dimension;
  // Array of item pointers
  Item<T>** items;

public:
  BruteForce (interface::Data<T>& ds) {
    // Initialize data
    imageCount = ds.n;
    dimension = ds.dimension;
    items = ds.items;
  }

  ~BruteForce () {

  }

  /*
  Method to calculate the averageDistance between a portion of the items.
  This is used to calculate a "proper" window size for LSH & Hypercube
  */
  double averageDistance (double datasetPercentage) {
    // Calculate the size of the partition
    int partitionSize = floor(datasetPercentage*imageCount);

    // For each item inside the partition...
    double outerSum = 0.0;
    for (int i = 0; i < partitionSize; i++) {
      int innerSum = 0;
      // Traverse each other item inside the partition
      for (int j = 0; j < partitionSize; j++) {
        if (i == j) continue;
        // Calculate the distance between the two items (and summarize it)
        innerSum += metrics::ManhattanDistance<T>(items[i]->data, items[j]->data, dimension);
      }
      // Then calculate the average distance of 1 item from all other items
      outerSum += 1.0*innerSum/partitionSize;
    }

    // Then calculate the average distance of all items from all other items
    return outerSum/partitionSize;
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


    // For each item...
    for (int i = 0; i < imageCount; i++) {
      // Calculate its distance from the query item
      int distance = metrics::ManhattanDistance<T>(query, items[i]->data, dimension);

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
        d[N-1].second = items[i];
        std::sort(d.begin(), d.end(), comparePairs<T>);
      }

      /*
      If a certain threshold of items traversed is reached, return the vector.
      If thresh == 0 it indicates that the user does not want to add a threshold.
      */
      if (thresh != 0 && i >= thresh)
        return d;
    }

    return d;
  }

  /*
  The following function creates a KNNOutput object.
  This object contains all information required to create the output file.
  This is used only on ANN (not clustering) and also requires that its LSH or Hypercube
  counterpart will be executed (so as to compare times and distances).
  */
  void buildOutput (interface::output::KNNOutput& output, interface::Dataset& query, int N, int thresh = 0) {
    // A vector containing the ids of all query items
    std::vector<int> queryIdVec;
    // The true distances of the first k near neighbors from each query item
    std::vector<std::vector<double>> distVec;
    // The total time kNN took to calculate the k nearest neighbors
    std::vector<double> timeVec;

    // For each query item...
    for (int i = 0; i < query.number_of_images; i++) {
      // Initialize a temporary distance vector to be added into distVec
      std::vector<double> tmpDistVec;

      // Execute kNN and calculate time elapsed
      clock_t begin = clock();

      std::vector<std::pair<int, Item<T>*>> kNNRes = kNN(query.images[i], N, thresh);

      clock_t end = clock();
      double elapsed = double(end - begin) / CLOCKS_PER_SEC;

      // For every neighbor returned, add its distance to tmpDistVec
      for (int j = 0; j < kNNRes.size(); j++){
        if (!kNNRes[j].second->null){
          tmpDistVec.push_back( (double) kNNRes[j].first);
        }
      }

      // Push all information gained by this loop into the respective vectors
      distVec.push_back(tmpDistVec);
      timeVec.push_back(elapsed);
      queryIdVec.push_back(i);
    }

    // Set the following KNNOutput's attributes to the vectors created above
    output.true_distance = distVec;
    output.true_time = timeVec;
    output.query_id = queryIdVec;
  }
};
