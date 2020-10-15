#include <iostream>
#include <chrono>

#include "../include/interfaces/LSH_interface.h"
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

  std::cout << "Hi\n";

  /* parse LSH input */
  int ret = interface::input::LSH::LSHParseInput(argc, argv, lsh_input, files, status);

  /* parse dataset */
  int ret3 = interface::ParseDataset(files.input_file, data);

  /* parse query set */
  int ret4 = interface::ParseDataset(files.query_file, queries);


  /* demonstrate the structs and their fields */
  std::cout << "\nLSH Input:" << "\n\n";
  std::cout << "k = " << +lsh_input.k << ", L = " << +lsh_input.L << ", N = " << lsh_input.N << ", R = " << lsh_input.R << "\n\n";

  std::cout << "\nDataset:" << "\n";
  std::cout << "\nmagic number: " << data.magic_number << "\n# of images: " << data.number_of_images
            << "\nrows per image: " << data.rows_per_image << "\ncolumns per image: " << data.columns_per_image << std::endl;

  uint32_t area = data.rows_per_image * data.columns_per_image;

  for (int image = 0; image < 10; image++)
  {
    std::cout << "\nImage: " << image << "\n\n";
    for (int pixel = 0; pixel < area; pixel++)
    {
      std::cout << +data.images[image][pixel] << " ";
    }

    std::cout << "\n";
  }
  std::cout << "\n";

  std::cout << "\nQueryset:" << "\n";
  std::cout << "\nmagic number: " << queries.magic_number << "\n# of images: " << queries.number_of_images
            << "\nrows per image: " << queries.rows_per_image << "\ncolumns per image: " << queries.columns_per_image << std::endl;

  uint32_t area_2 = queries.rows_per_image * queries.columns_per_image;

  for (int image = 0; image < 10; image++)
  {
    std::cout << "\nImage: " << image << "\n\n";
    for (int pixel = 0; pixel < area_2; pixel++)
    {
      std::cout << +queries.images[image][pixel] << " ";
    }

    std::cout << "\n";
  }
  std::cout << "\n";

  return 0;
}
