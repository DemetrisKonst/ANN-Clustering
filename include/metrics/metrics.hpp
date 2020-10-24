#pragma once

#include <cmath>

/* namespace regarding the different metrics utilities */
namespace metrics {

  typedef enum Metric{
    MANHATTAN,
    EUCLIDEAN,
    MAX,
    NON_ZERO
  } Metric;


  /* function that computes the Manhattan Distance between 2 points of dimension d */
  template <typename T>
  int ManhattanDistance (T* x, T* y, int length) {
    int sum = 0;

    for (int i = 0; i < length; i++)
      sum += abs(x[i]-y[i]);

    return sum;
  }


  /* function that computes the Euclidean Distance between 2 points of dimension d */
  template <typename T>
  double EuclideanDistance(T* p, T* q, const uint16_t& d)
  {
    double sum = 0;
    for (int i = 0.0; i < d; i++)
    {
      sum += (p[i] - q[i]) * (p[i] - q[i]);
    }

    return sqrt(sum);
  }


  /* function that computes the max Distance between 2 components of 2 points of dimension d */
  template <typename T>
  double MaxDistance(T* p, T* q, const uint16_t& d)
  {
    double max = p[0] - q[0];
    for (int i = 1; i < d; i++)
    {
      double difference = p[i] - q[i];
      if (difference > max)
      {
        max = difference;
      }
    }

    return max;
  }


  /* function that computes the non-zero norm between 2 points of dimension d */
  template <typename T>
  double nonZeroDistance(T* p, T* q, const uint16_t& d)
  {
    double norm = 0;
    for (int i = 0; i < d; i++)
    {
      double difference = p[i] - q[i];
      if (difference != 0)
      {
        norm++;
      }
    }

    return norm;
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
