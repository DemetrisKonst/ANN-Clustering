#include <iostream>
#include <chrono>
#include <utility>

#include "../include/interfaces/LSH_interface.h"
#include "../include/BruteForce/BruteForce.hpp"
#include "../include/LSH/LSH.hpp"

int main(int argc, char const *argv[]) {

  /* define useful variables */
  int success = 0;
  bool response = true;
  interface::ExitCode status;
  interface::Dataset dataset;
  interface::Dataset queries;
  interface::IOFiles files;
  interface::input::LSH::LSHInput lsh_input;
  interface::output::KNNOutput output;


  /* parse LSH input */
  success = interface::input::LSH::LSHParseInput(argc, argv, lsh_input, files, status);
  /* check for potential errors or violations */
  if (success != 1) {
    interface::output::PrintErrorMessageAndExit(status);
  }

  /* parse dataset */
  success = interface::ParseDataset(files.input_file, dataset, status);
  /* check for potential errors or violations */
  if (success != 1) {
    interface::output::PrintErrorMessageAndExit(status);
  }
  /* create a Data object used to store the data */
  interface::Data<uint8_t> data(dataset);


  // Initialize Brute Force
  BruteForce<uint8_t> bf = BruteForce<uint8_t>(data);

  // Calculate the window size (or set it to a default value)
  double averageItemDistance = utils::averageDistance<uint8_t>(0.05, data.items, data.n, data.dimension);
  int windowConstant = 1;
  int windowSize = (int) windowConstant*averageItemDistance;
  // int windowSize = 40000;
  // std::cout << "Window Size: " << windowSize << '\n';

  // Initialize LSH
  LSH<uint8_t> lsh = LSH<uint8_t>(lsh_input, data, windowSize);


  /* get the query set and and output file, in case they are not provided by the command line parameters */
  interface::ScanInput(files, status, false, files.query_file.empty(), files.output_file.empty());


  /* keep iterating while there is a new queryset to perform queries on */
  while (response) {

    /* parse the query set */
    success = interface::ParseDataset(files.query_file, queries, status);
    /* check for potential errors or violations */
    if (success != 1) {
      interface::output::PrintErrorMessageAndExit(status);
    }

    /* start building the output object */
    output.n = queries.number_of_images;
    output.method = "LSH";
    /* perform the queries for the brute force algorithm */
    bf.buildOutput(output, queries, lsh_input.N);
    /* perform the queries for the LSH algorithm */
    lsh.buildOutput(output, queries, lsh_input.N, lsh_input.R);

    /* write the results to the specified output file */
    interface::output::writeOutput(files.output_file, output, status);

    /* free the memory for the current query set */
    interface::freeDataset(queries);

    /* ask the user if he/she/it (it's 2020, we don't judge) wants to repeat the experiment */
    std::cout << "Would you like to to repeat the experiment with a different query set and output file? (y/n)\n";
    /* variable to store the answer */
    std::string answer;
    std::cin >> answer;

    /* check the response of the user */
    response = (answer == "y") || (answer == "Y") || (answer == "Yes") || (answer == "YES") || (answer == "yes");

    /* if a positive response was given */
    if (response) {
      /* get the names of the new files */
      interface::ScanInput(files, status, false, true, true);
    }

  }

  /* free the training dataset and return, as we have finished */
  interface::freeDataset(dataset);
  return 0;
}
