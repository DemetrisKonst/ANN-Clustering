#include <iostream>
#include <chrono>

#include "../include/interfaces/clustering_interface.h"
#include "../include/Clustering/clustering.hpp"


int main(int argc, char const *argv[]) {

  /* define the variables */
  interface::ExitCode status;
  interface::Dataset dataset;
  interface::IOCFiles files;
  interface::input::clustering::ClusteringInput cluster_input;
  interface::input::clustering::ClusteringConfig cluster_config;

  /* parse clustering input */
  int ret = interface::input::clustering::ClusteringParseInput(argc, argv, cluster_input, files, status);
  if (ret != 1)
  {
    interface::output::PrintErrorMessageAndExit(status);
  }
  ret = interface::input::clustering::ClusteringParseConfigFile(files.configuration_file, cluster_config, status);
  if (ret != 1)
  {
    interface::output::PrintErrorMessageAndExit(status);
  }

  /* parse dataset */
  int ret3 = interface::ParseDataset(files.input_file, dataset);

  /////////////////////// SMALLER DATASET ///////////////////////
  interface::Dataset dataset2;
  dataset2.magic_number = dataset.magic_number;
  dataset2.number_of_images = dataset.number_of_images / 10;
  dataset2.rows_per_image = dataset.rows_per_image;
  dataset2.columns_per_image = dataset.columns_per_image;

  dataset2.images = new uint8_t*[dataset2.number_of_images];
  for (int i = 0; i < dataset2.number_of_images; i++)
  {
    dataset2.images[i] = dataset.images[i];
  }
  /////////////////////// SMALLER DATASET ///////////////////////


  /* show input */
  // std::cout << "complete: " << cluster_input.complete << ", method: " << cluster_input.algorithm << std::endl;
  // std::cout << "\nClusters: " << cluster_config.clusters_K << ", L = " << +cluster_config.LSH_L << ", LSH_k = " << +cluster_config.LSH_k
  //           << ", HC_M = " << cluster_config.HC_M << ", HC_k = " << +cluster_config.HC_k << ", HC_probes = " << cluster_config.HC_probes << "\n\n";

  /* create a Data object that will be used to move around the data */
  interface::Data<uint8_t> data(dataset2);

  /* create a Clustering object in order to perform the clustering */
  clustering::Clustering<uint8_t> cluster(cluster_config, data);
  /* measure the time taken by the process */
  auto start = std::chrono::high_resolution_clock::now();
  cluster.perform_clustering(data, "Classic");
  auto end = std::chrono::high_resolution_clock::now();
  double duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

  /* get the silhouette and print it */
  double average_silhouette = cluster.compute_average_silhouette(data);

  /* create an Output object, and build it */
  interface::output::clustering::ClusteringOutput output;
  cluster.build_output(output, data, cluster_input, duration);

  /* log the results to the outfile */
  interface::output::clustering::writeOutput(files.output_file, output, status);


  // /* get the vectors that got mapped in cluster 3 */
  // uint16_t target_cluster = 3;
  // std::vector<Item<uint8_t>*>* vectors_in_cluster = cluster.get_vectors_in_cluster(target_cluster);
  //
  // /* print some of the images in it */
  // for (int i = 0; i < 10; i++)
  // {
  //   /* get the Item as a variable */
  //   Item<uint8_t>* item = (*vectors_in_cluster)[i];
  //   long int index = item->id;
  //
  //   /* log some information */
  //   std::cout << "Image: " << index << "\n\n";
  //   /* print the image */
  //   interface::output::printImage(item->data, dataset2.rows_per_image, dataset2.columns_per_image);
  //   std::cout << "\n\n";
  // }


  /* free up the allocated space and return */
  interface::freeDataset(dataset);
  cluster.free_output_object_memory(output);

  /////////////////////// SMALLER DATASET ///////////////////////
  delete[] dataset2.images;
  /////////////////////// SMALLER DATASET ///////////////////////

  return 0;
}
