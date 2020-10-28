#include <iostream>
#include <chrono>
#include <utility>

#include "../include/interfaces/LSH_interface.h"
#include "../include/BruteForce/BruteForce.hpp"
#include "../include/LSH/LSH.hpp"

int main(int argc, char const *argv[]) {

  /* define the variables */
  interface::ExitCode status;
  interface::Dataset dataset;
  interface::Dataset queries;
  interface::IOFiles files;
  interface::input::LSH::LSHInput lsh_input;
  // interface::input::HCUBE::HCUBE_input hcube_input;

  /* parse LSH input */
  int ret = interface::input::LSH::LSHParseInput(argc, argv, lsh_input, files, status);

  /* parse dataset */
  int ret3 = interface::ParseDataset(files.input_file, dataset);
  interface::Data<uint8_t> data(dataset);

  int ret4 = interface::ParseDataset(files.query_file, queries);

  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  BruteForce<uint8_t> bf = BruteForce<uint8_t>(data);
  std::vector<std::vector<std::pair<int, Item<uint8_t>*>>> bfOutput = bf.buildOutput(queries, lsh_input.N);

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "BF Time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;


  begin = std::chrono::steady_clock::now();

  LSH<uint8_t> lsh = LSH<uint8_t>(lsh_input, data);
  std::vector<std::vector<std::pair<int, Item<uint8_t>*>>> lshOutput = lsh.buildOutput(queries, lsh_input.N);

  end = std::chrono::steady_clock::now();
  std::cout << "LSH Time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

  interface::output::

  return 0;
}
