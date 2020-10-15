#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <arpa/inet.h>

#include "../../include/interfaces/interface.h"


void interface::ScanInput(interface::IOFiles& files, interface::ExitCode& status, const bool& change_dataset, const bool& change_query, const bool& change_output)
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
      GetValidFilename(files.input_file, "dataset", true);
    }
  }


  /* if a query file has not been specified */
  if (files.query_file.empty())
  {
    GetValidFilename(files.query_file, "query", true);
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
      GetValidFilename(files.query_file, "query", true);
    }
  }


  /* if an output file has not been specified */
  if (files.output_file.empty())
  {
    GetValidFilename(files.output_file, "output", true);
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
      GetValidFilename(files.output_file, "output", true);
    }
  }
}


int interface::ParseDataset(const std::string& filename, interface::Dataset& dataset)
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
      input_file.read((char *) &dataset.images[i][pixel], sizeof(dataset.images[i][pixel]));
    }
  }

  /* everything is done, return */
  return 1;
}
