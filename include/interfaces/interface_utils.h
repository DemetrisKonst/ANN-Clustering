#ifndef __INTERFACE_UTILS__
#define __INTERFACE_UTILS__


/* namespace regarding interface utilities */
namespace interface
{

  /* different reason for why the program could abort at any time */
  typedef enum ExitCode
  {
    FAIL,
    SUCCESS,
    NO_INPUT,
    INVALID_INPUT_LSH,
    INVALID_INPUT_HCUBE,
    HELP_MSG_LSH,
    HELP_MSG_HCUBE,
    INVALID_K,
    INVALID_L,
    INVALID_M,
    INVALID_PROBES,
    INVALID_N,
    INVALID_R,
    INVALID_INFILE_PATH,
    INVALID_QFILE_PATH,
    INVALID_OUTFILE_PATH
  } ExitCode;

  /* function to check if a file exists from its path */
  bool FileExists(const std::string& path);

  /* function that checks if a file can be opened */
  bool FileIsAccessible(const std::string& path);

  /* function to read a filename that performs a loop until a valid
     (existing or accessible, depending on param existance) filename is given */
  void GetValidFilename(std::string& filename, const std::string& type, const bool& existance);


  /* namespace regarding output with the user */
  namespace output
  {
    /* function that shows how an LSH input should be formed */
    void LSHPrintInputFormat(void);

    /* function that shows how a HYPER CUBE input should be formed */
    void HCUBEPrintInputFormat(void);

    /* function to print an error message and then exit */
    void PrintErrorMessageAndExit(const interface::ExitCode& code);
  }
}


#endif
