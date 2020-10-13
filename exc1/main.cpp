#include <iostream>
#include <cmath>
#include <random>
#include <chrono>

#include "../include/LSH/LSH.hpp"
#include "../include/BruteForce/BruteForce.hpp"

int main(int argc, char const *argv[]) {
  uint8_t** data;
  int trainingData = 10000;
  int d = 784;
  double radius = 60000.0;

  data = new uint8_t*[trainingData];
  for (int i = 0; i < trainingData; i++)
    data[i] = new uint8_t[d];

  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  for (int i = 0; i < trainingData; i++){
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<uint8_t> distribution(0, 255);

    for (int j = 0; j < d; j++){
      data[i][j] = distribution(generator);
    }
  }

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  std::cout << "Time difference of data creation = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;


  begin = std::chrono::steady_clock::now();

  LSH lsh (trainingData, 16, d, 4, 4, radius, pow(2, 32) - 5, data);
  BruteForce bf (trainingData, d, data);

  end = std::chrono::steady_clock::now();
  std::cout << "Time difference of Hashing = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;

  int testData = 10;

  uint8_t** tdata;

  tdata = new uint8_t*[testData];
  for (int i = 0; i < testData; i++)
    tdata[i] = new uint8_t[d];


  for (int i = 0; i < testData; i++){
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<uint8_t> distribution(0, 255);

    for (int j = 0; j < d; j++){
      tdata[i][j] = distribution(generator);
    }
  }


  begin = std::chrono::steady_clock::now();

  for (int i = 0; i < testData; i++){
    std::cout << "\n-------------" << i << "-----------------\n";
    std::vector<std::pair<int, uint8_t*>> res = lsh.ApproxNN(tdata[i], 5);
    std::pair<int, uint8_t*> resBF = bf.NearestNeighbor(tdata[i]);

    for (int j = 0; j < res.size(); j++){
      std::cout << "-" << j << " -> " << res[j].first << " -> " << (int)res[j].second[0] << "\n";
    }
    std::cout << "--BF Distance " << resBF.first << " -> " << (int)resBF.second[0] << "\n";
  }

  end = std::chrono::steady_clock::now();
  std::cout << "Time difference of kNN = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;


  begin = std::chrono::steady_clock::now();

  for (int i = 0; i < testData; i++){
    std::cout << "\n-------------" << i << "-----------------\n";
    std::vector<std::pair<int, uint8_t*>> res = lsh.RangeSearch(tdata[i], radius);

    for (int j = 0; j < res.size(); j++){
      std::cout << "--" << j << " +> Distance " << res[j].first << "\n";
    }
  }

  end = std::chrono::steady_clock::now();
  std::cout << "Time difference of Range Search = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;

  return 0;
}
