#include <utility>
#include <limits>

#include "../metrics/metrics.hpp"

template <typename T>
class BruteForce {
private:
  T** data;
  int N;
  int D;

public:
  BruteForce (int n, int d, T** data) : data(data), N(n), D(d) {}

  std::pair<int, T*> NearestNeighbor (T* query) {
    int b = std::numeric_limits<int>::max();
    T* d = NULL;

    for (int i = 0; i < N; i++) {
      int distance = metrics::ManhattanDistance<T>(data[i], query, D);

      if (distance < b) {
        b = distance;
        d = data[i];
      }
    }

    return std::make_pair(b, d);
  }

  std::pair<int, T*> RangeSearch (T* query, double radius) {
    for (int i = 0; i < N; i++) {
      int distance = metrics::ManhattanDistance<T>(data[i], query, D);

      if (distance < radius) {
        return std::make_pair(distance, data[i]);
      }
    }
  }
};
