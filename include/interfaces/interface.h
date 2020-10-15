#ifndef __INTERFACE__
#define __INTERFACE__


#include <cstdlib>
#include <cstdint>

#include "interface_utils.h"


/* namespace regarding interface utilities */
namespace interface
{

  /* struct used to move around the IO values */
  typedef struct IOFiles
  {
    std::string input_file = "";
    std::string query_file = "";
    std::string output_file = "";
  } IOFiles;

  /* struct used to define the contents of a dataset */
  typedef struct Dataset
  {
    uint32_t magic_number = 0;
    uint32_t number_of_images = 0;
    uint32_t rows_per_image = 0;
    uint32_t columns_per_image = 0;
    uint8_t** images = NULL;
  } Dataset;


  /* function used to scan the query and output files, in case they were not passed as command line parameters */
  void ScanInput(IOFiles& files, ExitCode& status, const bool& change_dataset, const bool& change_query, const bool& change_output);

  /* function used to read input from the dataset */
  int ParseDataset(const std::string& filename, Dataset& dataset);
}


#endif