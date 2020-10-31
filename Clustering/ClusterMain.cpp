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

  /* create a Clustering object in order to perform the clustering */
  clustering::Clustering<uint8_t> cluster(cluster_config, data);
  /* perform the clustering */
  cluster.perform_clustering(data, cluster_input.algorithm, &duration);

  /* get the silhouette and print it */
  double average_silhouette = cluster.compute_average_silhouette(data);

  /* create an Output object, build it and use it to log the results to the outfile */
  interface::output::clustering::ClusteringOutput output;
  cluster.build_output(output, data, cluster_input, duration);
  interface::output::clustering::writeOutput(files.output_file, output, status);


  /* free up the allocated space and return */
  interface::freeDataset(dataset);
  cluster.free_output_object_memory(output);
  return 0;
}
