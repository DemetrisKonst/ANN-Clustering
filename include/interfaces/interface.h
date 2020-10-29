#ifndef _INTERFACE
#define _INTERFACE


#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <vector>
#include <utility>
#include <algorithm>
#include <limits>

#include "interface_utils.h"
#include "../core/item.hpp"


/* namespace regarding interface utilities */
namespace interface
{

  /* struct used to group the IO (input - output) values */
  typedef struct IOFiles
  {
    std::string input_file = "";
    std::string query_file = "";
    std::string output_file = "";
  } IOFiles;

  /* struct used to group the IOC (input - output - configuration) values */
  typedef struct IOCFiles
  {
    std::string input_file = "";
    std::string configuration_file = "";
    std::string output_file = "";
  } IOCFiles;

  /* struct used to define the contents of a dataset */
  typedef struct Dataset
  {
    uint32_t magic_number = 0;
    uint32_t number_of_images = 0;
    uint32_t rows_per_image = 0;
    uint32_t columns_per_image = 0;
    uint8_t** images = NULL;
  } Dataset;


  /* struct used to move around the Dataset */
  template <typename T>
  struct Data
  {
    uint32_t n = 0;
    uint16_t dimension = 0;
    Item<T>** items = NULL;

    Data(Dataset& dataset)
    {
      /* initialize the values of the Data */
      n = dataset.number_of_images;
      dimension = dataset.rows_per_image * dataset.columns_per_image;
      items = new Item<T>*[n];

      /* create each item separately */
      for (int i = 0; i < n; i++)
      {
        items[i] = new Item<T>(i, dataset.images[i], false, false);
      }
    }

    ~Data(void)
    {
      /* free each item separately */
      for (int i = 0; i < n; i++)
      {
        delete items[i];
      }

      /* now delete the items array */
      delete[] items;
      items = NULL;

    }
  };


  /* function used to scan the query and output files, in case they were not passed as command line parameters */
  void ScanInput(IOFiles& files, ExitCode& status, const bool& change_dataset, const bool& change_query, const bool& change_output);

  /* function used to read input from the dataset */
  int ParseDataset(const std::string& filename, Dataset& dataset, ExitCode& status);

  /* function to free up the memory that the dataset used */
  void freeDataset(Dataset& dataset);


  /* namespace regarding output utilities */
  namespace output
  {
    /* struct used to group the output data */
    typedef struct KNNOutput
    {
      uint32_t n;
      std::string method;
      std::vector<int> query_id;    // item id
      std::vector<std::vector<int>> n_neighbors_id;     // knn neighbors item id
      std::vector<std::vector<double>> approx_distance; // knn distance
      std::vector<std::vector<double>> true_distance;   // real distance
      std::vector<double> approx_time;  //time to complete kNN for 1 query
      std::vector<double> true_time;
      std::vector<std::vector<int>> r_near_neighbors_id; // item id of range search neighbors
    } KNNOutput;

    /* function to write the output of LSH/HC to a file */
    int writeOutput(const std::string& outfile_name, KNNOutput& output, ExitCode& status);
  }

}


#endif
