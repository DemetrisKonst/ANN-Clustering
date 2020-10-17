#pragma once

#include <cmath>

namespace metrics {

  int ManhattanDistance (uint8_t* x, uint8_t* y) {
    int sum = 0;

    for (int i = 0; i < 784; i++)
      sum += abs(x[i]-y[i]);

    return sum;
  }

  int HammingdDistance (int x, int y) {
    int a = x ^ y;
    int setBits = 0;

    while (a > 0) {
        setBits += a & 1;
        a >>= 1;
    }

    return setBits;
  }

}
