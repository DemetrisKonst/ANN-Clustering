#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <regex>
#include <iomanip>

#include "../../include/interfaces/clustering_interface.h"


int interface::input::clustering::ClusteringParseInput(const int& argc, const char* argv[], interface::input::clustering::ClusteringInput& input, interface::IOCFiles& files, interface::ExitCode& status)
{
  /* first check that at least some parameters have been given */
  if (argc == 1)
  {
    status = NO_INPUT_CLUSTERING;
    return 0;
  }

  /* at least 5 parameters should be given: 1) program name, 2/3) input file flag/input, 4/5) configuration file flag/input */
  if (argc < 5)
  {
    /* now check if the option "--help" has been given */
    if (!strcmp(argv[1], "--help"))
    {
      status = HELP_MSG_CLUSTERING;
      return 0;
    }

    /* if we get here it means the option "help" was not given, so act accordingly */
    status = INVALID_INPUT_CLUSTERING;
    return 0;
  }


  /* keep flags to make sure that the input file (dataset), the configuration file and the clustering method are provided */
  bool flag_input_file = false;
  bool flag_config_file = false;
  bool flag_method = false;

  /* iterate through the arguments to find the values */
  int i = 1;
  while (i < argc)
  {
    /* get the option provided by the user */
    std::string option(argv[i]);

    /* check if a flag that expectes a value has been given, but no value is provided */
    if (option == "-d" || option == "-c" || option == "-o" || option == "-m")
    {
      if (i + 1 == argc)
      {
        status = INVALID_INPUT_CLUSTERING;
        return 0;
      }
    }

    /* enumerate the command line parameters */
    if (option == "-d")
    {
      std::string value(argv[i + 1]);
      if (!FileExists(value))
      {
        status = INVALID_INFILE_PATH;
        return 0;
      }
      flag_input_file = true;
      files.input_file = value;
      i++;
    }
    else if (option == "-c")
    {
      std::string value(argv[i + 1]);
      if (!FileExists(value))
      {
        status = INVALID_CONFIG_PATH;
        return 0;
      }
      flag_config_file = true;
      files.configuration_file = value;
      i++;
    }
    else if (option == "-o")
    {
      std::string value(argv[i + 1]);
      if (!FileIsAccessible(value))
      {
        status = INVALID_OUTFILE_PATH;
        return 0;
      }
      files.output_file = value;
      i++;
    }
    else if (option == "-complete")
    {
      input.complete = true;
    }
    else if (option == "-m")
    {
      std::string value(argv[i + 1]);
      if (value != "Classic" && value != "LSH" && value != "Hypercube")
      {
        status = INVALID_CLUSTERING_METHOD;
        return 0;
      }
      flag_method = true;
      input.algorithm = value;
      i++;
    }
    else
    {
      status = INVALID_INPUT_CLUSTERING;
      return 0;
    }

    i++;
  }

  /* check if the input file (dataset) was not given */
  if (!flag_input_file || !flag_config_file || !flag_method)
  {
    status = NO_INPUT_CLUSTERING;
    return 0;
  }

  /* everything went ok, return */
  return 1;
}


int interface::input::clustering::ClusteringParseConfigFile(const std::string& filename, interface::input::clustering::ClusteringConfig& config, interface::ExitCode& status)
{
  /* create in ifstream object to read the input from the configuration file */
  std::ifstream config_file(filename);

  /* make sure that the file successfully opened */
  if (!config_file.is_open())
  {
    status = INVALID_CONFIG_PATH;
    return 0;
  }

  /* create a variable to read lines from the file */
  std::string line = "";

  /* create a regex to match all integer values */
  std::regex reg("(-?[0-9]+)");

  /* create an std::smatch object to match regexes */
  std::smatch matches;


  /* field 1: number_of_clusters: <int>                    // K of K-means */
  std::getline(config_file, line);
  /* search matches */
  std::regex_search(line, matches, reg);

  /* check if a value for the number of clusters has been given */
  if (!matches.empty())
  {
    int value_as_int = stoi(matches.str(1));
    /* check for the correct range of values */
    if (value_as_int <= 1 || value_as_int >= 256)
    {
      status = CONFIG_INVALID_CLUSTERS;
      return 0;
    }
    config.clusters_K = value_as_int;
  }
  /* else, raise an error because this field is mandatory */
  else
  {
    status = CONFIG_NO_CLUSTERS;
    return 0;
  }


  /* field 2: number_of_vector_hash_tables: <int>          // default: L = 3 */
  std::getline(config_file, line);
  /* search matches */
  std::regex_search(line, matches, reg);

  /* check if a value has been given; else assume the default value */
  if (!matches.empty())
  {
    int value_as_int = stoi(matches.str(1));
    /* check for the correct range of values */
    if (value_as_int <= 0 || value_as_int >= 256)
    {
      status = INVALID_L;
      return 0;
    }
    config.LSH_L = value_as_int;
  }


  /* field 3: number_of_vector_hash_functions: <int>       // k of LSH for vectors, default: 4 */
  std::getline(config_file, line);
  /* search matches */
  std::regex_search(line, matches, reg);

  /* check if a value has been given; else assume the default value */
  if (!matches.empty())
  {
    int value_as_int = stoi(matches.str(1));
    /* check for the correct range of values */
    if (value_as_int <= 0 || value_as_int >= 256)
    {
      status = INVALID_K;
      return 0;
    }
    config.LSH_k = value_as_int;
  }


  /* field 4: max_number_M_hypercube: <int>                // M of Hypercube, default: 10 */
  std::getline(config_file, line);
  /* search matches */
  std::regex_search(line, matches, reg);

  /* check if a value has been given; else assume the default value */
  if (!matches.empty())
  {
    int value_as_int = stoi(matches.str(1));
    /* check for the correct range of values */
    if (value_as_int <= 0)
    {
      status = INVALID_M;
      return 0;
    }
    config.HC_M = value_as_int;
  }


  /* field 5: number_of_hypercube_dimensions: <int>        // k of Hypercube, default: 3 */
  std::getline(config_file, line);
  /* search matches */
  std::regex_search(line, matches, reg);

  /* check if a value has been given; else assume the default value */
  if (!matches.empty())
  {
    int value_as_int = stoi(matches.str(1));
    /* check for the correct range of values */
    if (value_as_int <= 0)
    {
      status = INVALID_K_DIM;
      return 0;
    }
    config.HC_k = value_as_int;
  }


  /* field 6: number_of_probes: <int>                      // probes of Hypercube, default: 2 */
  std::getline(config_file, line);
  /* search matches */
  std::regex_search(line, matches, reg);

  /* check if a value has been given; else assume the default value */
  if (!matches.empty())
  {
    int value_as_int = stoi(matches.str(1));
    /* check for the correct range of values */
    if (value_as_int <= 0)
    {
      status = INVALID_PROBES;
      return 0;
    }
    config.HC_probes = value_as_int;
  }

  /* everythind is done, close the file and return */
  config_file.close();
  return 1;
}


int interface::output::clustering::writeOutput(const std::string& outfile_name, const interface::output::clustering::ClusteringOutput& output, interface::ExitCode& status)
{
  /* create in ifstream object to open the output file */
  std::ofstream outfile;
  outfile.open(outfile_name, std::ios::out | std::ios::trunc);

  /* make sure that the file successfully opened */
  if (!outfile.is_open())
  {
    status = INVALID_OUTFILE_PATH;
    return 0;
  }

  /* log information about algorithm used */
  outfile << "Algorithm: ";
  if (output.method == "Classic")
  {
    outfile << "Lloyds";
  }
  else if (output.method == "LSH")
  {
    outfile << "Range Search LSH";
  }
  else
  {
    outfile << "Range Search Hypercube";
  }
  outfile << std::endl;

  /* log information about clusters */
  for (int c = 0; c < output.K; c++)
  {
    outfile << "Cluster-" << c + 1 << " {size: " << output.cluster_sizes[c] << " centroid: [";
    for (int j = 0; j < output.d; j++)
    {
      outfile << +output.centroids[c][j];
      if (j < output.d - 1)
      {
        outfile << " ";
      }
    }
    outfile << "]}" << std::endl;
  }

  /* log information about clustering time */
  outfile << "clustering_time: " << output.clustering_time << std::endl;

  /* log information about silhouettes */
  outfile << "Silhouette: [";
  for (int c = 0; c < output.K; c++)
  {
    outfile << output.cluster_silhouettes[c] << ", ";
  }
  outfile << output.total_silhouette << "]" << std::endl;


  /* check if the -complete flag was given */
  if (output.complete)
  {
    /* if yes, print the centroid and the images in it of each cluster */
    outfile << std::endl;
    for (int c = 0; c < output.K; c++)
    {
      outfile << "Cluster-" << c + 1 << " {[";
      for (int j = 0; j < output.d; j++)
      {
        outfile << +output.centroids[c][j];
        if (j < output.d - 1)
        {
          outfile << " ";
        }
      }
      outfile << "], ";
      for (int i = 0; i < output.items[c]->size(); i++)
      {
        Item<uint8_t>* item = (*output.items[c])[i];
        outfile << item->id;
        if (i < output.items[c]->size() - 1)
        {
          outfile << ", ";
        }
      }
      outfile << "}" << std::endl;
    }

  }

  /* everything is done, close the outfile and return */
  outfile.close();
  return 1;
}
