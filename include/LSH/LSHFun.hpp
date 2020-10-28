#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

#include "../utils/numbers.hpp"

template <typename T>
class HashFunction {
  double searchRadius;      //search radius given by user
  int windowConstant;       // constant to get window = windowConstant*searchRadius
  int functionAmount;                    // number of H functions for g
  int dimension;                    // dimension
  int window;               // window = windowConstant*searchRadius >> searchRadius
  int modularConstant;      // modularConstant = 2^(32/functionAmount)

  int* randomShift;
  int* mmodM_values;

public:
  HashFunction (double sr, int c, int k, int d, int* mmod):
  searchRadius(sr), windowConstant(c), functionAmount(k), dimension(d), mmodM_values(mmod) {
    window = windowConstant*searchRadius;
    modularConstant = pow(2, 32/functionAmount);

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, window);

    randomShift = new int[dimension];

    for (int i = 0; i < dimension; i++){
      randomShift[i] = distribution(generator);
    }
  }

  int HashVector (T* x) {
    std::vector<int> a;   // vector storing floor((Xi-Si)/window)

    unsigned long long int sum = 0;

    for (int i = 0; i < dimension; i++) {
      a.push_back(floor((x[i]-randomShift[i])*1.0/window));

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
  double searchRadius;
  int windowConstant;            // constant to get window = windowConstant*searchRadius
  int functionAmount;            // number of H functions for g
  int dimension;            // dimension
  int window;

public:
  AmplifiedHashFunction (double sr, int c, int k, int d, int* mmod):
  searchRadius(sr), windowConstant(c), functionAmount(k), dimension(d){
    window = windowConstant*searchRadius;

    for (int i = 0; i < functionAmount; i++) {
      H.push_back(HashFunction<T>(sr, windowConstant, functionAmount, d, mmod));
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
