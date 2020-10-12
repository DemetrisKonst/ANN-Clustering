#include <iostream>
#include <cmath>
#include <random>
#include <chrono>

#include "../include/LSH/LSHFun.hpp"

int main(int argc, char const *argv[]) {
  AmplifiedHashFunction lshf(1.0, 10, 6, 784, pow(2, 32) - 5);
  AmplifiedHashFunction lshf2(1.0, 10, 6, 784, pow(2, 32) - 5);
  AmplifiedHashFunction lshf3(1.0, 10, 6, 784, pow(2, 32) - 5);
  AmplifiedHashFunction lshf4(1.0, 10, 6, 784, pow(2, 32) - 5);


  int** data;
  data = new int*[60000];
  for (int i = 0; i < 60000; i++)
    data[i] = new int[784];

  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  for (int i = 0; i < 60000; i++){
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, 255);

    for (int j = 0; j < 784; j++){
      data[i][j] = distribution(generator);
    }
  }

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  std::cout << "Time difference of data creation = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;


  begin = std::chrono::steady_clock::now();

  for (int i = 0; i < 60000; i++){
    unsigned long int gres = lshf.HashVector(data[i]);
    unsigned long int gres2 = lshf2.HashVector(data[i]);
    unsigned long int gres3 = lshf3.HashVector(data[i]);
    unsigned long int gres4 = lshf4.HashVector(data[i]);
  }

  end = std::chrono::steady_clock::now();

  std::cout << "Time difference of Hashing = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;

  return 0;
}
