#include <utility>
#include <limits>

#include "../metrics/metrics.hpp"

class BruteForce {
private:
  uint8_t** data;
  int N;
  int D;

public:
  BruteForce (int n, int d, uint8_t** data) : data(data), N(n), D(d) {

  }

  std::pair<int, uint8_t*> NearestNeighbor (uint8_t* query) {
    int b = std::numeric_limits<int>::max();
    uint8_t* d = NULL;

    for (int i = 0; i < N; i++) {
      int distance = metrics::ManhattanDistance(data[i], query);

      if (distance < b) {
        b = distance;
        d = data[i];
      }
    }

    return std::make_pair(b, d);
  }

  std::pair<int, uint8_t*> RangeSearch (uint8_t* query, double radius) {
    for (int i = 0; i < N; i++) {
      int distance = metrics::ManhattanDistance(data[i], query);

      if (distance < radius) {
        return std::make_pair(distance, data[i]);
      }
    }
  }
};
