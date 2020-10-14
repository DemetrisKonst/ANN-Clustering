#ifndef __LSH_INTERFACE__
#define __LSH_INTERFACE__

#include <cstdlib>
#include <cstdint>

/* namespace regarding interface utilities */
namespace interface
{

  /* different reason for why the program could abort at any time */
  typedef enum ExitCode
  {
    FAIL,
    SUCCESS,
    NO_INPUT,
    INVALID_INPUT,
    HELP_MSG,
    INVALID_K,
    INVALID_L,
    INVALID_N,
    INVALID_R,
    INVALID_INFILE_PATH,
    INVALID_QFILE_PATH,
    INVALID_OUTFILE_PATH
  } ExitCode;

  /* function to print an error message and then exit */
  void LHSPrintErrorMessageAndExit(const interface::ExitCode& code);

  /* function to check if a file exists from its path */
  bool FileExists(const std::string& path);

  /* function that checks if a file can be opened */
  bool FileIsAccessible(const std::string& path);

  /* function to read a filename that performs a loop until a valid
     (existing or accessible, depending on param existance) filename is given */
  void GetValidFilename(std::string& filename, const std::string& type, const bool& existance);

  /* namespace regarding command line parameters utilities */
  namespace input
  {
    /* struct used to define the LSH input (from the command line), the rest (epsilon, c) can be found in a configfile */
    typedef struct LSH_input
    {
      uint8_t k = 4;
      uint8_t L = 5;
      uint32_t N = 1;
      double R = 1.0;
      std::string input_file = "";
      std::string query_file = "";
      std::string output_file = "";
    } LSH_input;

    /* struct used to define the contents of a dataset */
    typedef struct LSH_dataset
    {
      uint32_t magic_number = 0;
      uint32_t number_of_images = 0;
      uint32_t rows_per_image = 0;
      uint32_t columns_per_image = 0;
      uint8_t** images = NULL;
    } LSH_dataset;

    /* function that shows how an input could be formed */
    void LSHPrintInputFormat(void);

    /* function to parse command line parameters */
    int LSHParseInput(const int& argc, const char* argv[], LSH_input& input, ExitCode& status);

    /* function used to scan the query and output files, in case they were not passed as command line parameters */
    void LSHScanInput(LSH_input& input, ExitCode& status, const bool& change_dataset, const bool& change_query, const bool& change_output);

    /* function used to read input from the dataset */
    int LSHParseDataset(const std::string& filename, LSH_dataset& dataset);
  }



}


#endif
