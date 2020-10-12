#include <iostream>
#include <string>

namespace utils {
  unsigned long int concatenateNumbers (std::vector<int> n) {
    std::string finalStr = std::to_string(n[0]);

    for (int i = 1; i < n.size(); i++) {
      std::string tmpStr = std::to_string(n[i]);
      finalStr += tmpStr;
    }

    unsigned long int finalNum = stol(finalStr);


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
