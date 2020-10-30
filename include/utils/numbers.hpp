#pragma once

namespace utils {
  /*
  Helper function which takes in a vector of uint8_t and concatenates them bitwise
  to return an unsigned int.
  Used in AmplifiedHashFunction in LSHFun.hpp
  */
  unsigned int concatenateBitwise (std::vector<uint8_t> n) {
    unsigned int finalNum = n[0];

    for (int i = 1; i < n.size(); i++) {
      finalNum <<= 8;
      finalNum |= n[i];
    }

    return finalNum;
  }

  /*
  Helper function which takes in a base, a power and a constant
  to calculate (base^power) mod M. It uses the modular exponentiation algorithm to avoid overflow.
  Used in LSH & Hypercube to calculate the mmodM values
  */
  int modEx (unsigned long int base, unsigned long int power, int M) {
    if (power == 0){
      return 1;
    }else if (power%2 == 0){
      int tmp = modEx(base, power/2, M);
      return  (tmp*tmp)%M;
    }else{
      return ((base%M)*(modEx(base, power-1, M)))%M;
    }
  }
}
