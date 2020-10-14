#pragma once

namespace utils {
  unsigned long int concatenateBitwise (std::vector<uint8_t> n) {
    unsigned long int finalNum = n[0];

    for (int i = 1; i < n.size(); i++) {
      finalNum <<= 8;
      finalNum |= n[i];
    }

    return finalNum;
  }

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
