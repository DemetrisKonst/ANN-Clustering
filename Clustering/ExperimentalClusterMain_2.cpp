#include <iostream>

#include "../include/Clustering/clustering.hpp"


int main(int argc, char const *argv[]) {

  /* define useful variables */
  int success = 0;
  double duration = 0.0;
  interface::ExitCode status;
  interface::Dataset dataset;
  interface::IOCFiles files;
  interface::input::clustering::ClusteringInput cluster_input;
  interface::input::clustering::ClusteringConfig cluster_config;

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


  /* define the variables for the experiment */
  int epochs = 25;
  double silhouette = 0.0;
  double total_classic_duration = 0.0;
  double total_classic_silhouette = 0.0;
  double total_lsh_duration = 0.0;
  double total_lsh_silhouette = 0.0;
  double total_hc_duration = 0.0;
  double total_hc_silhouette = 0.0;

  /* compute the experiment for default K = 10, epoch number of times */
  std::cout << std::endl;
  for (int i = 0; i < epochs; i++)
  {
    /* keep a variable for the duration of the epoch */
    double epoch_duration = 0.0;

    /* create a Clustering object in order to perform the clustering */
    clustering::Clustering<uint8_t> cluster(cluster_config, data);

    /* perform the clustering with Lloyds algorithm */
    cluster.perform_clustering(data, "Classic", &duration);
    /* increment the durations */
    epoch_duration += duration;
    total_classic_duration += duration;
    /* get the silhouette */
    silhouette = cluster.compute_average_silhouette(data);
    total_classic_silhouette += silhouette;
    /* reset the cluster so that LSH and Hypercube can run */
    cluster.reset_clusters();

    /* perform the clustering with LSH algorithm */
    cluster.perform_clustering(data, "LSH", &duration);
    /* increment the durations */
    epoch_duration += duration;
    total_lsh_duration += duration;
    /* get the silhouette */
    silhouette = cluster.compute_average_silhouette(data);
    total_lsh_silhouette += silhouette;
    /* reset the cluster so that Hypercube can run */
    cluster.reset_clusters();

    /* perform the clustering with Hypercube algorithm */
    cluster.perform_clustering(data, "Hypercube", &duration);
    /* increment the durations */
    epoch_duration += duration;
    total_hc_duration += duration;
    /* get the silhouette */
    silhouette = cluster.compute_average_silhouette(data);
    total_hc_silhouette += silhouette;
    /* reset the cluster for no reason whatsoever */
    cluster.reset_clusters();


    /* log some information */
    std::cout << "Duration of epoch " << i + 1 << " for clustering: " << epoch_duration << std::endl;
  }

  /* compute the average values */
  double average_classic_execution_time = ((double) total_classic_duration) / epochs;
  double average_classic_silhouette = ((double) total_classic_silhouette) / epochs;
  double average_lsh_execution_time = ((double) total_lsh_duration) / epochs;
  double average_lsh_silhouette = ((double) total_lsh_silhouette) / epochs;
  double average_hc_execution_time = ((double) total_hc_duration) / epochs;
  double average_hc_silhouette = ((double) total_hc_silhouette) / epochs;

  /* print some newlines*/
  std::cout << std::endl << std::endl;

  /* print info about Lloyds algorithm */
  std::cout << "Classic Algorithm Statistics:" << std::endl << std::endl;
  std::cout << "Average Classic Execution Time: " << average_classic_execution_time << std::endl;
  std::cout << "Average Classic Silhouette: " << average_classic_silhouette << std::endl << std::endl;

  /* print info about LSH algorithm */
  std::cout << "LSH Algorithm Statistics:" << std::endl << std::endl;
  std::cout << "Average LSH Execution Time: " << average_lsh_execution_time << std::endl;
  std::cout << "Average LSH Silhouette: " << average_lsh_silhouette << std::endl << std::endl;

  /* print info about Hypercube algorithm */
  std::cout << "Hypercube Algorithm Statistics:" << std::endl << std::endl;
  std::cout << "Average Hypercube Execution Time: " << average_hc_execution_time << std::endl;
  std::cout << "Average Hypercube Silhouette: " << average_hc_silhouette << std::endl << std::endl;


  /* free up the allocated space and return */
  interface::freeDataset(dataset);
  return 0;
}
