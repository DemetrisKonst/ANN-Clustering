#include <cmath>

namespace metrics {

  int ManhattanDistance (int* x, int* y) {
    int sum = 0;
    
    for (int i = 0; i < 784; i++)
      sum += abs(x[i]-y[i]);

    return sum;
  }

}
