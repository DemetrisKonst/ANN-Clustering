#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

#include "../utils/numbers.hpp"

/*
This class is responsible for hashing items into an integer value.
The return value is parameterized by a constant W (windowSize) given as
an argument into this class' constructor.
At first each feature is shifted by "s". S is a random value in [0, W).
Those S values remain constant for each instance of this class (they are initialized on creation)
Then the shifted feature is divided by W.
Aftewards, the "n"th feature is multiplied by m^(dimension-n-1).
All of those are then summarized into one single value.
Finally, the integer returned is the result of "sum_value" mod M.
M=2^(32/k) where k is the amount of HashFunctions selected in the algorithm
*/
template <typename T>
class HashFunction {
  // number of H functions used in the algorithm
  int functionAmount;
  // dimension of items
  int dimension;
  // the windowSize (W) inserted by the user
  int windowSize;
  // modularConstant (M) is the value which is used to calculate the modulo of the sum
  int modularConstant;

  // Array of S values initialized at creation S->[0, W)
  int* randomShift;
  // Array of m^(dimension-n-1)modM values, initialized outside of this class as they are constant
  int* mmodM_values;

public:
  HashFunction (int ws, int k, int d, int* mmod) : windowSize(ws), functionAmount(k), dimension(d), mmodM_values(mmod) {
    // Calculate M
    modularConstant = pow(2, 32/functionAmount);

    // Initialize S values
    randomShift = new int[dimension];

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, windowSize-1);

    for (int i = 0; i < dimension; i++){
      randomShift[i] = distribution(generator);
    }
  }

  ~HashFunction () {
    delete[] randomShift;
  }

  /*
  The following method hashes the vector by calculating the aforementioned sum.
  It is calculated through modular exponentiation (as the huge values would cause an overflow).
  Modular exponentiation is only used when calculating the (m^k)modM values (outside of this class).
  */
  int HashVector (T* x) {
    // The following vector stores the shifted values divided by windowSize
    std::vector<int> a;

    unsigned int sum = 0;
    for (int i = 0; i < dimension; i++) {
      a.push_back(floor((x[i]-randomShift[i])*1.0/windowSize));

      /*
      We need to calculate the modulo of each of these values since
      we are using the following formula to not achieve overflow:
      (a$b)modM = ((amodM)$(bmodM))modM.
      $ represents {+, -, *, /}
      */
      // Calculate a mod M
      int amodm = a[i]%modularConstant;
      // Calculate (amodM*mmodM)modM
      int resmodm = (amodm * mmodM_values[i])%modularConstant;
      // Add the above into the sum
      sum += resmodm;
    }

    // Finally perform modulo on the sum to return the desired value
    return sum%modularConstant;
  }
};

/*
The following class implements an amplified hash function.
An amplified hash function is a hash function which initializes k HashFunction objects.
Then, whenever an item is to be hashed, it fetches the hash values from those k HashFunctions.
Finally, those values are concatenated bitwise to produce the final hash value for the item.
*/
template <typename T>
class AmplifiedHashFunction {
private:
  // Number of H functions
  int functionAmount;
  // Data dimension
  int dimension;
  // Window size entered by the user
  int windowSize;

  // Array of HashFunctions
  HashFunction<T>** H;

public:
  AmplifiedHashFunction (int ws, int k, int d, int* mmod) : windowSize(ws), functionAmount(k), dimension(d){
    // Initialize k HashFunctions
    H = new HashFunction<T>*[functionAmount];
    for (int i = 0; i < functionAmount; i++) {
      H[i] = new HashFunction<T>(windowSize, functionAmount, dimension, mmod);
    }
  }

  ~AmplifiedHashFunction () {
    for (int i = 0; i < functionAmount; i++)
      delete H[i];
    delete[] H;
  }

  unsigned int HashVector (T* x) {
    // Initialize a vector of values returned by each HashFunction
    std::vector<uint8_t> hValues;

    // Calculate the values and insert them to the vector
    for (int i = 0; i < functionAmount; i++) {
      hValues.push_back(H[i]->HashVector(x));
    }

    // Produce the final value by concatenating bitwise the values of the array
    unsigned int concatRes = utils::concatenateBitwise(hValues);

    return concatRes;
  }
};
