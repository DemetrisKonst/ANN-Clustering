#include <iostream>
#include <chrono>

#include "../include/interfaces/clustering_interface.h"
#include "../include/Clustering/clustering.hpp"


int main(int argc, char const *argv[]) {

  /* define the variables */
  interface::ExitCode status;
  interface::Dataset data;
  interface::IOCFiles files;
  interface::input::clustering::clustering_input cluster_input;
  interface::input::clustering::clustering_config cluster_config;

  /* parse clustering input */
  int ret = interface::input::clustering::clusteringParseInput(argc, argv, cluster_input, files, status);
  if (ret != 1)
  {
    interface::output::PrintErrorMessageAndExit(status);
  }
  ret = interface::input::clustering::clusteringParseConfigFile(files.configuration_file, cluster_config, status);
  if (ret != 1)
  {
    interface::output::PrintErrorMessageAndExit(status);
  }

  /* parse dataset */
  int ret3 = interface::ParseDataset(files.input_file, data);

  /* show input */
  std::cout << "complete: " << cluster_input.complete << ", method: " << cluster_input.algorithm << std::endl;
  std::cout << "\nClusters: " << cluster_config.clusters_K << ", L = " << +cluster_config.LSH_L << ", LSH_k = " << +cluster_config.LSH_k
            << ", HC_M = " << cluster_config.HC_M << ", HC_k = " << +cluster_config.HC_k << ", HC_probes = " << cluster_config.HC_probes << "\n\n";


  clustering::Clustering<uint8_t> cluster(cluster_config, data);
  cluster.perform_clustering(data, "Classic");

  clustering::ClusterCenter<uint8_t>** centers = cluster.get_centers();

  /* print images from center 0 */
  for (int i = 0; i < 10; i++)
  {
    std::cout << "Image: " << i << "\n\n";
    uint32_t index = centers[0]->vectors_in_cluster[0][i];
    interface::output::printImage(data.images[index], data.rows_per_image, data.columns_per_image);
    std::cout << "\n\n";
  }



  // /* print some images */
  // for (int i = 0; i < 10; i++)
  // {
  //   std::cout << "Image: " << i << "\n\n";
  //   interface::output::printImage(data.images[i], data.rows_per_image, data.columns_per_image);
  //   std::cout << "\n\n";
  // }


  /* free up the allocated space and return */
  interface::freeDataset(data);
  return 0;
}
