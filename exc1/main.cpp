#include <iostream>

#include "../include/interfaces/LSH/LSH_interface.h"

int main(int argc, char const *argv[]) {

  interface::ExitCode status;

  interface::input::LSH_input input;
  interface::input::LSH_dataset data;

  int ret = interface::input::LSHParseInput(argc, argv, input, status);
  int ret2 = interface::input::LSHParseDataset(input.input_file, data);

  return 0;
}
