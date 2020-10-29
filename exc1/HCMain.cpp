#include <iostream>
#include <chrono>
#include <utility>
#include <cmath>

#include "../include/interfaces/HC_interface.h"

#include "../include/Hypercube/Hypercube.hpp"
#include "../include/BruteForce/BruteForce.hpp"


int main(int argc, char const *argv[]) {

  /* define the variables */
  interface::ExitCode status;
  interface::Dataset dataset;
  interface::Dataset queries;
  interface::IOFiles files;
  interface::input::HC::HCInput hc_input;
  // interface::input::HCUBE::HCUBE_input hcube_input;

  /* parse LSH input */
  int ret = interface::input::HC::HCParseInput(argc, argv, hc_input, files, status);

  /* parse dataset */
  int ret3 = interface::ParseDataset(files.input_file, dataset);
  interface::Data<uint8_t> data(dataset);
  /* parse query set */
  int ret4 = interface::ParseDataset(files.query_file, queries);

  interface::output::KNNOutput output;
  output.n = 5;
  output.R = 0.0;
  output.method = "Hypercube";

  BruteForce<uint8_t> bf = BruteForce<uint8_t>(data);
  // clock_t begin = clock();
  // double averageItemDistance = bf.averageDistance(0.05);
  // clock_t end = clock();
  // double elapsed = double(end - begin) / CLOCKS_PER_SEC;
  // std::cout << "avgItemDist: " << averageItemDistance << '\n';
  // std::cout << "Time for avgItemDist: " << elapsed << '\n';
  bf.buildOutput(output, queries, hc_input.N);

  Hypercube<uint8_t> hc = Hypercube<uint8_t>(hc_input, data, 10000);
  hc.buildOutput(output, queries, hc_input.N, hc_input.R, hc_input.probes, hc_input.M);

  interface::output::writeOutput(files.output_file, output, status);

  return 0;
}
