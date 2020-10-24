#pragma once

#include <cmath>

namespace metrics {

  template <typename T>
  int ManhattanDistance (T* x, T* y, int length) {
    int sum = 0;

    for (int i = 0; i < length; i++)
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
