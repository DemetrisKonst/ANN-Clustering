#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

#include "../utils/numbers.hpp"

class HashFunction {
  double searchRadius;      //search radius given by user
  int c;                    // constant to get W = c*searchRadius
  int k;                    // number of H functions for g
  int D;                    // dimension
  int W;                    // W = c*searchRadius >> searchRadius
  unsigned long int m;
  int M;

  int* s;
  int* mmod;

public:
  HashFunction (double sr, int c, int k, int d, unsigned long int m) : searchRadius(sr), c(c), k(k), D(d), m(m) {
    W = c*searchRadius;
    M = pow(2, 32/k);

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, W);

    s = new int[D];
    mmod = new int[D];

    for (int i = 0; i < D; i++){
      s[i] = distribution(generator);
      mmod[i] = utils::modEx(m, D-i-1, M);
    }
  }

  HashFunction (double sr, int c, int k, int d, unsigned long int m, int* mmod) : searchRadius(sr), c(c), k(k), D(d), m(m), mmod(mmod) {
    W = c*searchRadius;
    M = pow(2, 32/k);

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, W);

    s = new int[D];

    for (int i = 0; i < D; i++){
      s[i] = distribution(generator);
    }
  }

  int HashVector (int* x) {
    std::vector<int> a;   // vector storing floor((Xi-Si)/W)

    unsigned long long int sum = 0;

    for (int i = 0; i < D; i++) {
      a.push_back(floor((x[i]-s[i])*1.0/W));

      int amodm = a[i]%M;
      int resmodm = (amodm * mmod[i])%M;

      sum += resmodm;
    }

    return sum%M;
  }
};

class AmplifiedHashFunction {
private:
  std::vector<HashFunction> H;
  double searchRadius;
  int c;            // constant to get W = c*searchRadius
  int k;            // number of H functions for g
  int D;            // dimension
  int W;
  unsigned long int m;
  int M;

public:
  AmplifiedHashFunction (double sr, int c, int k, int d, unsigned long int m) : searchRadius(sr), c(c), k(k), D(d), m(m)  {
    W = c*searchRadius;
    M = pow(2, 32/k);

    int* mmod = new int[d];

    for (int i = 0; i < D; i++)
      mmod[i] = utils::modEx(m, D-i-1, M);

    for (int i = 0; i < k; i++) {
      H.push_back(HashFunction(sr, c, k, d, m, mmod));
    }
  }

  unsigned long int HashVector (int* x) {
    unsigned long int g;

    std::vector<int> hValues;

    for (int i = 0; i < k; i++) {
      hValues.push_back(H[i].HashVector(x));
    }

    // std::random_shuffle(hValues.begin(), hValues.end());

    unsigned long int concatRes = utils::concatenateNumbers(hValues);

    return concatRes;
  }
};
