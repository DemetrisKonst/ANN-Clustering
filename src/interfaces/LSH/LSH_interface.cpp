#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <arpa/inet.h>

#include "../../../include/interfaces/LSH/LSH_interface.h"


void interface::LHSPrintErrorMessageAndExit(const interface::ExitCode& code)
{
  /* print an error according to the code */
  switch (code)
  {
    case NO_INPUT:
    {
      std::cerr << "ERROR: No input was given by the user."
                << "At least the path of the input file (dataset) has to be provided in the command line parameters.\n";

      break;
    }
    case INVALID_INPUT:
    {
      std::cerr << "ERROR: command line arguments cannot be recognized. Input is invalid."
                << "Consult below on how to execute the program.\n";
      input::LSHPrintInputFormat();
      break;
    }
    case HELP_MSG:
    {
      input::LSHPrintInputFormat();
      break;
    }
    case INVALID_K:
    {
      std::cerr << "ERROR: The value passed for the number of hash functions k is invalid."
                << "Usually k is a positive int from 4 to 6.\n";
      break;
    }
    case INVALID_L:
    {
      std::cerr << "ERROR: The value passed for the number of hast tables L is invalid."
                << "Usually L is either 5 or 6.\n";
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
    default:
    {
      std::cerr << "ERROR: LSH_interface::Whoops this shouldn't have happened. ELOUSA.\n";
      break;
    }
  }

  exit(EXIT_FAILURE);
}


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
  /* return true if it succeeds; else false */
  return (file) ? true : false;
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


void interface::input::LSHPrintInputFormat(void)
{
  std::cout << "The input from the command line parameters should be something like this:\n\n"
            << "$ ./lsh -d <input_file> [-q <query_file>] [-k <int>] [-o <output_file>] [-N <number of nearest>] [-R <radius>]\n\n"
            << "Note that for the default values are k=4, L=5, N=1, R=1.0. The query and output files must be provided by the user at some point.\n"
            << "You can type: $ ./lsh --help, to print this information at any time.\n";
}


int interface::input::LSHParseInput(const int& argc, const char* argv[], interface::input::LSH_input& input, interface::ExitCode& status)
{
  /* first check that at least some parameters have been given */
  if (argc == 1)
  {
    status = NO_INPUT;
    return 0;
  }

  /* now check if the option "--help" has been given, or if the input length implies a mistake */
  if (argc > 13 || argc % 2 == 0)
  {
    /* make sure that the option "--help" was given */
    if (!strcmp(argv[1], "--help"))
    {
      status = HELP_MSG;
      return 0;
    }

    /* if we get here it means the option was not given, so act accordingly */
    status = INVALID_INPUT;
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
      input.input_file = value;
    }
    else if (option == "-q")
    {
      if (!FileExists(value))
      {
        status = INVALID_QFILE_PATH;
        return 0;
      }
      input.query_file = value;
    }
    else if (option == "-k")
    {
      input.k = stoi(value);
    }
    else if (option == "-L")
    {
      input.L = stoi(value);
    }
    else if (option == "-o")
    {
      if (!FileIsAccessible(value))
      {
        status = INVALID_OUTFILE_PATH;
        return 0;
      }
      input.output_file = value;
    }
    else if (option == "-N")
    {
      input.N = std::stoul(value, NULL);
    }
    else if (option == "-R")
    {
      input.R = stof(value);
    }
    else
    {
      status = NO_INPUT;
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


void interface::input::LSHScanInput(interface::input::LSH_input& input, interface::ExitCode& status, const bool& change_dataset, const bool& change_query, const bool& change_output)
{
  /* check if the user wants to change dataset */
  if (change_dataset)
  {
    /* ask the user if we wants to provide a new filename */
    std::cout << "Would you like to change the dataset file? (y/n)\n";
    std::string answer;
    std::cin >> answer;

    /* check the answer of the user */
    if (answer == "y" || answer == "yes" || answer == "Y" || answer == "YES")
    {
      GetValidFilename(input.input_file, "dataset", true);
    }
  }


  /* if a query file has not been specified */
  if (input.query_file.empty())
  {
    GetValidFilename(input.query_file, "query", true);
  }
  /* else, if we want to change it */
  else if (change_query)
  {
    /* ask the user if we wants to provide a new filename */
    std::cout << "Would you like to change the query file? (y/n)\n";
    std::string answer;
    std::cin >> answer;

    /* check the answer of the user */
    if (answer == "y" || answer == "yes" || answer == "Y" || answer == "YES")
    {
      GetValidFilename(input.query_file, "query", true);
    }
  }


  /* if an output file has not been specified */
  if (input.output_file.empty())
  {
    GetValidFilename(input.output_file, "output", true);
  }
  /* else, if we want to change it */
  else if (change_output)
  {
    /* ask the user if we wants to provide a new filename */
    std::cout << "Would you like to change the output file? (y/n)\n";
    std::string answer;
    std::cin >> answer;

    /* check the answer of the user */
    if (answer == "y" || answer == "yes" || answer == "Y" || answer == "YES")
    {
      GetValidFilename(input.output_file, "output", true);
    }
  }
}


int interface::input::LSHParseDataset(const std::string& filename, interface::input::LSH_dataset& dataset)
{
  /* create an ifstream item to open and navigate the file */
  std::ifstream input_file(filename, std::ios::binary);

  /* make sure that the file successfully opened */
  if (!input_file.is_open())
  {
    return 0;
  }

  /* temp variables to read input from the file */
  uint32_t temp_big_endian;
  uint8_t temp_pixel;

  /* read the magic number, number of images and their dimensions */
  input_file.read((char *) &(temp_big_endian), sizeof(temp_big_endian));
  dataset.magic_number = ntohl(temp_big_endian);

  input_file.read((char *) &(temp_big_endian), sizeof(temp_big_endian));
  dataset.number_of_images = ntohl(temp_big_endian);

  input_file.read((char *) &(temp_big_endian), sizeof(temp_big_endian));
  dataset.rows_per_image = ntohl(temp_big_endian);

  input_file.read((char *) &(temp_big_endian), sizeof(temp_big_endian));
  dataset.columns_per_image = ntohl(temp_big_endian);


  /* compute the "area" of an image to avoid computing it again */
  uint32_t area = dataset.rows_per_image * dataset.columns_per_image;

  /* initialize the images (pixels array) */
  dataset.images = new uint8_t*[dataset.number_of_images];

  /* iterate through the array to allocate space for the pixels of each image, while reading the image at the same time */
  for (int i = 0; i < dataset.number_of_images; i++)
  {
    dataset.images[i] = new uint8_t[area];
    /* add the pixels */
    for (uint32_t pixel = 0; pixel < area; pixel++)
    {
      input_file.read((char *) &temp_pixel, sizeof(temp_pixel));
      dataset.images[i][pixel] = temp_pixel;
    }
  }

  /* everything is done, return */
  return 1;
}
