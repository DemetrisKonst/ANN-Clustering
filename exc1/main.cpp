#include <iostream>
#include <chrono>

#include "../include/interfaces/LSH/LSH_interface.h"
#include "../include/LSH/LSH.hpp"
#include "../include/BruteForce/BruteForce.hpp"


int main(int argc, char const *argv[]) {

  interface::ExitCode status;

  interface::input::LSH_input input;
  interface::input::LSH_dataset dataset;

  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  int ret = interface::input::LSHParseInput(argc, argv, input, status);
  int ret2 = interface::input::LSHParseDataset(input.input_file, dataset);

  std::cout << dataset.magic_number << "\n";

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "Time of Input Parsing = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;




  begin = std::chrono::steady_clock::now();

  LSH lsh (dataset.number_of_images, 16, dataset.rows_per_image*dataset.columns_per_image, input.k, input.L, input.R, pow(2, 32) - 5, dataset.images);

  end = std::chrono::steady_clock::now();
  std::cout << "Time of LSH Initialization = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;

  BruteForce bf (dataset.number_of_images, dataset.rows_per_image*dataset.columns_per_image, dataset.images);


  std::vector<std::pair<int, uint8_t*>> reskNN = lsh.ApproxNN(dataset.images[0], input.N);
  std::pair<int, uint8_t*> resBF = bf.NearestNeighbor(dataset.images[0]);

  for (int j = 0; j < reskNN.size(); j++){
    std::cout << "-" << j << " -> " << reskNN[j].first << " -> " << (int)reskNN[j].second[0] << "\n";
  }
  std::cout << "--BF Distance " << resBF.first << " -> " << (int)resBF.second[0] << "\n";

  std::vector<std::pair<int, uint8_t*>> resRS = lsh.RangeSearch(dataset.images[0], input.R);

  for (int j = 0; j < resRS.size(); j++){
    std::cout << "--" << j << " +> Distance " << resRS[j].first << "\n";
  }

  return 0;
}
// =======
// #include <cmath>
// #include <random>
// #include <chrono>

// #include "../include/LSH/LSH.hpp"
// #include "../include/BruteForce/BruteForce.hpp"

// int main(int argc, char const *argv[]) {
//   uint8_t** data;
//   int trainingData = 10000;
//   int d = 784;
//   double radius = 60000.0;

//   int testData = 10;

//   uint8_t** tdata;

//   tdata = new uint8_t*[testData];
//   for (int i = 0; i < testData; i++)
//     tdata[i] = new uint8_t[d];




//   begin = std::chrono::steady_clock::now();

//   for (int i = 0; i < testData; i++){
//     std::cout << "\n-------------" << i << "-----------------\n";
//     std::vector<std::pair<int, uint8_t*>> res = lsh.ApproxNN(tdata[i], 5);
//     std::pair<int, uint8_t*> resBF = bf.NearestNeighbor(tdata[i]);

//     for (int j = 0; j < res.size(); j++){
//       std::cout << "-" << j << " -> " << res[j].first << " -> " << (int)res[j].second[0] << "\n";
//     }
//     std::cout << "--BF Distance " << resBF.first << " -> " << (int)resBF.second[0] << "\n";
//   }

//   end = std::chrono::steady_clock::now();
//   std::cout << "Time difference of kNN = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;


//   begin = std::chrono::steady_clock::now();

//   for (int i = 0; i < testData; i++){
//     std::cout << "\n-------------" << i << "-----------------\n";
//     std::vector<std::pair<int, uint8_t*>> res = lsh.RangeSearch(tdata[i], radius);

//     for (int j = 0; j < res.size(); j++){
//       std::cout << "--" << j << " +> Distance " << res[j].first << "\n";
//     }
//   }

//   end = std::chrono::steady_clock::now();
//   std::cout << "Time difference of Range Search = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
// >>>>>>> main

//   return 0;
// }
