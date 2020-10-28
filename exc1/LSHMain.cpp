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

  interface::output::KNNOutput output;
  output.n = 5;
  output.R = 0.0;
  output.method = "LSH";

  BruteForce<uint8_t> bf = BruteForce<uint8_t>(data);
  bf.buildOutput(output, queries, lsh_input.N);

  LSH<uint8_t> lsh = LSH<uint8_t>(lsh_input, data);
  lsh.buildOutput(output, queries, lsh_input.N, lsh_input.R);

  interface::output::writeOutput(files.output_file, output, status);

  return 0;
}
