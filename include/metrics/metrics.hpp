#pragma once

#include <cmath>

namespace metrics {

  int ManhattanDistance (uint8_t* x, uint8_t* y) {
    int sum = 0;

    for (int i = 0; i < 784; i++)
      sum += abs(x[i]-y[i]);

    return sum;
  }

}
