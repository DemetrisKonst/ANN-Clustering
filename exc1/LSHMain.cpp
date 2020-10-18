#include <iostream>
#include <chrono>
#include <utility>

#include "../include/interfaces/LSH_interface.h"

#include "../include/LSH/LSH.hpp"
#include "../include/BruteForce/BruteForce.hpp"


int main(int argc, char const *argv[]) {

  /* define the variables */
  interface::ExitCode status;
  interface::Dataset data;
  interface::Dataset queries;
  interface::IOFiles files;
  interface::input::LSH::LSH_input lsh_input;
  // interface::input::HCUBE::HCUBE_input hcube_input;

  /* parse LSH input */
  int ret = interface::input::LSH::LSHParseInput(argc, argv, lsh_input, files, status);

  /* parse dataset */
  int ret3 = interface::ParseDataset(files.input_file, data);

  LSH lshmain = LSH(data.number_of_images, 16, 28*28, lsh_input.k, lsh_input.L, lsh_input.R, pow(2,32)-5, data.images);

  /* parse query set */
  int ret4 = interface::ParseDataset(files.query_file, queries);

  for (int i = 0; i < 5; i++) {
    std::vector<std::pair<int, uint8_t*>> kNNRes = lshmain.ApproxNN(queries.images[i], lsh_input.N);

    std::cout << "----" << i << "----" << '\n';
    for (int j = 0; j < kNNRes.size(); j++) {
      std::cout << j << "th Distance - ApproxNN " << kNNRes[j].first << '\n';
    }

    std::vector<std::pair<int, uint8_t*>> rsRes = lshmain.RangeSearch(queries.images[i], lsh_input.R);

    std::cout << "----" << i << "----" << '\n';
    for (int j = 0; j < rsRes.size(); j++) {
      std::cout << j << "th Distance - RangeSearch" << rsRes[j].first << '\n';
    }

  }

  return 0;
}
