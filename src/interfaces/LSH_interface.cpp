#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>

#include "../../include/interfaces/LSH_interface.h"



int interface::input::LSH::LSHParseInput(const int& argc, const char* argv[], interface::input::LSH::LSHInput& input, interface::IOFiles& files, interface::ExitCode& status)
{
  /* first check that at least some parameters have been given */
  if (argc == 1)
  {
    status = NO_INPUT;
    return 0;
  }

  /* now check if the option "--help" has been given, or if the input length implies a mistake */
  if (argc % 2 == 0)
  {
    /* make sure that the option "--help" was given */
    if (!strcmp(argv[1], "--help"))
    {
      status = HELP_MSG_LSH;
      return 0;
    }

    /* if we get here it means the option was not given, so act accordingly */
    status = INVALID_INPUT_LSH;
    return 0;
  }

  /* keep a flag to make sure that the input file (dataset) is provided */
  bool flag_input_file = false;

  /* start iterating the arguments array */
  for (int i = 1; i < argc; i += 2)
  {
    /* get the parameter and the value */
    std::string option(argv[i]);
    std::string value(argv[i+1]);

    if (option == "-d")
    {
      flag_input_file = true;
      if (!FileExists(value))
      {
        status = INVALID_INFILE_PATH;
        return 0;
      }
      files.input_file = value;
    }
    else if (option == "-q")
    {
      if (!FileExists(value))
      {
        status = INVALID_QFILE_PATH;
        return 0;
      }
      files.query_file = value;
    }
    else if (option == "-k")
    {
      int value_as_int = stoi(value);
      if (value_as_int <= 0 || value_as_int >= 32)
      {
        status = INVALID_K;
        return 0;
      }
      input.k = value_as_int;
    }
    else if (option == "-L")
    {
      int value_as_int = stoi(value);
      if (value_as_int <= 0)
      {
        status = INVALID_L;
        return 0;
      }
      input.L = value_as_int;
    }
    else if (option == "-o")
    {
      if (!FileIsAccessible(value))
      {
        status = INVALID_OUTFILE_PATH;
        return 0;
      }
      files.output_file = value;
    }
    else if (option == "-N")
    {
      int value_as_int = stoi(value);
      if (value_as_int <= 0)
      {
        status = INVALID_N;
        return 0;
      }
      input.N = value_as_int;
    }
    else if (option == "-R")
    {
      double value_as_double = stof(value);
      if (value_as_double <= 0)
      {
        status = INVALID_R;
        return 0;
      }
      input.R = value_as_double;
    }
    else
    {
      status = INVALID_INPUT_LSH;
      return 0;
    }

  }

  /* check if the input file (dataset) was not given */
  if (flag_input_file == false)
  {
    status = NO_INPUT;
    return 0;
  }

  /* everything went ok, return */
  return 1;
}
