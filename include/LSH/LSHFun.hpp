#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

#include "../utils/numbers.hpp"

template <typename T>
class HashFunction {
  double averageDistance;
  int windowConstant;       // constant to get windowSize = windowConstant*averageDistance
  int functionAmount;       // number of H functions for g
  int dimension;            // dimension
  int windowSize;               // windowSize = windowConstant*averageDistance >> averageDistance
  int modularConstant;      // modularConstant = 2^(32/functionAmount)

  int* randomShift;
  int* mmodM_values;

public:
  HashFunction (double avg, int c, int k, int d, int* mmod):
  averageDistance(avg), windowConstant(c), functionAmount(k), dimension(d), mmodM_values(mmod) {
    windowSize = (int) windowConstant*averageDistance;
    modularConstant = pow(2, 32/functionAmount);

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, windowSize);

    randomShift = new int[dimension];

    for (int i = 0; i < dimension; i++){
      randomShift[i] = distribution(generator);
    }
  }

  int HashVector (T* x) {
    std::vector<int> a;   // vector storing floor((Xi-Si)/windowSize)

    unsigned long long int sum = 0;

    for (int i = 0; i < dimension; i++) {
      a.push_back(floor((x[i]-randomShift[i])*1.0/windowSize));

      int amodm = a[i]%modularConstant;
      int resmodm = (amodm * mmodM_values[i])%modularConstant;

      sum += resmodm;
    }

    return sum%modularConstant;
  }
};

template <typename T>
class AmplifiedHashFunction {
private:
  std::vector<HashFunction<T>> H;
  double averageDistance;
  int windowConstant;            // constant to get windowSize = windowConstant*averageDistance
  int functionAmount;            // number of H functions for g
  int dimension;            // dimension
  int windowSize;

public:
  AmplifiedHashFunction (double avg, int c, int k, int d, int* mmod):
  averageDistance(avg), windowConstant(c), functionAmount(k), dimension(d){
    windowSize = windowConstant*averageDistance;

    for (int i = 0; i < functionAmount; i++) {
      H.push_back(HashFunction<T>(averageDistance, windowConstant, functionAmount, dimension, mmod));
    }
  }

  unsigned long int HashVector (T* x) {
    std::vector<uint8_t> hValues;

    for (int i = 0; i < functionAmount; i++) {
      hValues.push_back(H[i].HashVector(x));
    }

    // std::random_shuffle(hValues.begin(), hValues.end());

    unsigned long int concatRes = utils::concatenateBitwise(hValues);

    return concatRes;
  }
};
