#include <iostream>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "../include/Clustering/clustering.hpp"



/* helper method to get current datetime in a readable format */
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}



int main(int argc, char const *argv[]) {

  /* define useful variables */
  int success = 0;
  double duration = 0.0;
  double average_silhouette = 0.0;
  interface::ExitCode status;
  interface::Dataset dataset;
  interface::IOCFiles files;
  interface::input::clustering::ClusteringInput cluster_input;
  interface::input::clustering::ClusteringConfig cluster_config;
  interface::output::clustering::ClusteringOutput cluster_output;

  /* parse clustering input */
  success = interface::input::clustering::ClusteringParseInput(argc, argv, cluster_input, files, status);
  /* check for potential errors or violations */
  if (success != 1) {
    interface::output::PrintErrorMessageAndExit(status);
  }

  /* parse dataset */
  success = interface::ParseDataset(files.input_file, dataset, status);
  /* check for potential errors or violations */
  if (success != 1) {
    interface::output::PrintErrorMessageAndExit(status);
  }

  /* parse configuration file */
  success = interface::input::clustering::ClusteringParseConfigFile(files.configuration_file, cluster_config, status);
  /* check for potential errors or violations */
  if (success != 1) {
    interface::output::PrintErrorMessageAndExit(status);
  }


  /* create a Data object that will be used to move around the data */
  interface::Data<uint8_t> data(dataset);
  /* number of clusters */
  uint16_t K = 8;
  /* until where to carry the experiment */
  uint16_t max_K = 12;

  /* loop to try all the values */
  while (K <= max_K)
  {
    /* log some information */
    std::cout << "Starting K = " << K << " at: " << currentDateTime() << std::endl;
    /* set the number of clusters */
    cluster_config.clusters_K = K;
    /* set the outfile correctly */
    std::string outfile_path = "output/clustering_experiment/K_";
    std::string outfile = outfile_path + std::to_string(K);


    /* create a clustering object in order to perform the clustering */
    clustering::Clustering<uint8_t> cluster(cluster_config, data);

    /* create an array that stores the names of the methods used for clustering */
    uint8_t number_of_methods = 3;
    const char* methods[number_of_methods] = {"Classic", "LSH", "Hypercube"};

    /* iterate through all the methods available */
    for (int i = 0; i < number_of_methods; i++)
    {
      /* have the method as a string */
      std::string method(methods[i]);

      /* perform the clustering with Lloyds algorithm */
      cluster.perform_clustering(data, method, &duration);
      /* compute the average silhouette */
      average_silhouette = cluster.compute_average_silhouette(data);

      /* build the output object and write the output in the logfile */
      cluster_input.algorithm = method;
      cluster.build_output(cluster_output, data, cluster_input, duration);
      interface::output::clustering::writeOutput(outfile + "_" + method, cluster_output, status);
      cluster.free_output_object_memory(cluster_output);

      /* now reset the cluster back to the normal state so that the next method can be perfomed */
      cluster.reset_clusters();
    }

    /* increment the number of clusters */
    K++;
  }


  /* free up the allocated space and return */
  interface::freeDataset(dataset);

  /* log some information */
  std::cout << "Finished at: " << currentDateTime() << std::endl;

  return 0;
}
