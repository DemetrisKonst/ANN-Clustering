#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <arpa/inet.h>

#include "../../include/interfaces/interface_utils.h"


bool interface::FileExists(const std::string& path)
{
  /* create a path variable */
  struct stat buffer;
  /* check if the exists by trying access its stats */
  return (stat (path.c_str(), &buffer) == 0);
}


bool interface::FileIsAccessible(const std::string& path)
{
  /* try to create the file */
	std::ofstream file{path};
  /* create a variable that will be returned */
  bool is_accessible = (file) ? true : false;

  /* close the file */
  file.close();
  
  /* return true if it succeeds; else false */
  return is_accessible;
}


void interface::GetValidFilename(std::string& filename, const std::string& type, const bool& existance)
{
  /* while the filename is empty */
  while (1 + 2 == 3)
  {
    /* ask the user for input */
    std::cout << "Give the path of the " << type << " file.\n";
    std::string temp = "";
    std::cin >> temp;

    /* determine if we want to check for existance of accessibility */
    if (existance)
    {
      /* if it exists, assign it and return */
      if (FileExists(temp))
      {
        filename = temp;
        return;
      }

      /* else, repeat the process while notifying the user */
      std::cout << "Wrong input: filename " << temp << " can't be opened. Please give another name.\n";
    }
    else
    {
      /* if it is accessible, assign it and return */
      if (FileIsAccessible(temp))
      {
        filename = temp;
        return;
      }

      /* else, repeat the process while notifying the user */
      std::cout << "Wrong input: filename " << temp << " can't be accessed. Please give another name.\n";
    }
  }
}


void interface::output::printImage(const uint8_t* image_vector, const uint16_t& rows, const uint16_t& columns)
{
  /* for every row of the image */
  for (int row = 0; row < rows; row++)
  {
    /* for every column of the image */
    for (int column = 0; column < columns; column++)
    {
      /* print the image */
      printf("%3u ", image_vector[row * rows + column]);
    }
    /* go to the next line */
    std::cout << std::endl;
  }
}


void interface::output::LSHPrintInputFormat(void)
{
  std::cout << "The input from the command line parameters should be something like this:\n\n"
            << "$ ./lsh -d <input_file> [-q <query_file>] [-k <int>] [-L <int>] [-o <output_file>] [-N <number of nearest>] [-R <radius>]\n\n"
            << "Note that for the default values are k=4, L=5, N=1, R=1.0. The query and output files must be provided by the user at some point.\n"
            << "You can type: $ ./lsh --help, to print this information at any time.\n";
}


void interface::output::HCUBEPrintInputFormat(void)
{
  std::cout << "The input from the command line parameters should be something like this:\n\n"
            << "$ ./cube -d <input_file> [-q <query_file>] [-k <int>] [-M <int>] [-probes <int>] [-o <output_file>] [-N <number of nearest>] [-R <radius>]\n\n"
            << "Note that for the default values are k=3, M=10, probes=2, N=1, R=1.0. The query and output files must be provided by the user at some point.\n"
            << "You can type: $ ./cube --help, to print this information at any time.\n";
}


void interface::output::clusteringShowInputFormat(void)
{
  std::cout << "The input from the command line parameters should be something like this:\n\n"
            << "$ ./cluster -d <input_file> -c <configuration_file> -o <output_file> [-complete] -m <method>\n\n"
            << "You can type: $ ./cluster --help, to print this information at any time.\n";
}


void interface::output::PrintErrorMessageAndExit(const interface::ExitCode& code)
{
  std::cout << std::endl << std::endl;
  /* print an error according to the code */
  switch (code)
  {
    case NO_INPUT:
    {
      std::cerr << "ERROR: Not enough input was given by the user.\n"
                << "At least the path of the input file (dataset) has to be provided in the command line parameters.\n";
      break;
    }
    case NO_INPUT_CLUSTERING:
    {
      std::cerr << "ERROR: Not enough input was given by the user.\n"
                << "At least the path of the input file (dataset), the path of the configuration file and the Clustering method have to be provided in the command line parameters.\n";
      break;
    }
    case INVALID_INPUT_LSH:
    {
      std::cerr << "ERROR: command line arguments cannot be recognized. Input is invalid.\n"
                << "Consult below on how to execute the program.\n";
      output::LSHPrintInputFormat();
      break;
    }
    case INVALID_INPUT_HCUBE:
    {
      std::cerr << "ERROR: command line arguments cannot be recognized. Input is invalid.\n"
                << "Consult below on how to execute the program.\n";
      output::HCUBEPrintInputFormat();
      break;
    }
    case INVALID_INPUT_CLUSTERING:
    {
      std::cerr << "ERROR: command line arguments cannot be recognized. Input is invalid.\n"
                << "Consult below on how to execute the program.\n";
      output::clusteringShowInputFormat();
      break;
    }
    case HELP_MSG_LSH:
    {
      output::LSHPrintInputFormat();
      break;
    }
    case HELP_MSG_HCUBE:
    {
      output::HCUBEPrintInputFormat();
      break;
    }
    case HELP_MSG_CLUSTERING:
    {
      output::clusteringShowInputFormat();
      break;
    }
    case INVALID_K:
    {
      std::cerr << "ERROR: The value passed for the number of hash functions k is invalid.\n"
                << "Usually k is a positive int from 4 to 6 (but can range from 1 to 31).\n";
      break;
    }
    case INVALID_K_DIM:
    {
      std::cerr << "ERROR: The value passed for the dimension of the Hypercube (k = d') is invalid.\n"
                << "It should be a positive in between 0-255.\n";
      break;
    }
    case INVALID_L:
    {
      std::cerr << "ERROR: The value passed for the number of hast tables L is invalid."
                << "Usually L is either 5 or 6.\n";
      break;
    }
    case INVALID_M:
    {
      std::cerr << "ERROR: The value passed for the number of allowed candidate points to be checked, M, is invalid."
                << "It should be a positive integer.\n";
      break;
    }
    case INVALID_PROBES:
    {
      std::cerr << "ERROR: The value passed for maximum number of cube vertices to be checked, probes, is invalid."
                << "It should be a positive integer.\n";
      break;
    }
    case INVALID_N:
    {
      std::cerr << "ERROR: The value passed for the number of Approximate Nearest Neighbors N is invalid."
                << "It should be a positive integer.\n";
      break;
    }
    case INVALID_R:
    {
      std::cerr << "ERROR: The value passed for the radius r is invalid."
                << "It should be a positive double.\n";
      break;
    }
    case INVALID_CLUSTERING_METHOD:
    {
      std::cerr << "ERROR: The clustering method given is invalid.\nThe available clustering methods are: Classic, LSH, Hypercube.\n";
      break;
    }
    case CONFIG_INVALID_CLUSTERS:
    {
      std::cerr << "ERROR: The value of the number of clusters K is invalid.\nIt should be a positive integer, greater than 1 and smaller than 256.\n";
      break;
    }
    case CONFIG_NO_CLUSTERS:
    {
      std::cerr << "ERROR: In the configuration file, the first line should contain the number od clusters K.\n"
                << "It should be a positive value.\n";
      break;
    }
    case INVALID_INFILE_PATH:
    {
      std::cerr << "ERROR: The path passed for the input file (dataset) is invalid.\n";
      break;
    }
    case INVALID_QFILE_PATH:
    {
      std::cerr << "ERROR: The path passed for the query file is invalid.\n";
      break;
    }
    case INVALID_OUTFILE_PATH:
    {
      std::cerr << "ERROR: The path passed for the output file is invalid.\n";
      break;
    }
    case INVALID_CONFIG_PATH:
    {
      std::cerr << "ERROR: The path passed for the configuration file is invalid.\n";
      break;
    }
    default:
    {
      std::cerr << "ERROR: LSH_interface::Whoops this shouldn't have happened. ELOUSA.\n";
      break;
    }
  }

  std::cout << std::endl << std::endl;
  exit(EXIT_FAILURE);
}
