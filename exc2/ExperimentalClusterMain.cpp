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
  int K = 8;
  while (K <= 12)
  {
    std::cout << "Starting K = " << K << " at: " << currentDateTime() << std::endl;
    /* set the number of clusters */
    cluster_config.clusters_K = K;
    /* set the outfile correctly */
    std::string outfile_parent_dir = "output/clustering_experiment/K_";
    std::string outfile = outfile_parent_dir + std::to_string(K);


    /* create a Clustering object in order to perform the clustering */
    clustering::Clustering<uint8_t> cluster_classic(cluster_config, data);
    /* perform the clustering */
    cluster_classic.perform_clustering(data, "Classic", &duration);

    /* get the silhouette and print it */
    double average_silhouette = cluster_classic.compute_average_silhouette(data);

    /* create an Output object, build it and use it to log the results to the outfile */
    interface::output::clustering::ClusteringOutput output_classic;
    cluster_input.algorithm = "Classic";
    cluster_classic.build_output(output_classic, data, cluster_input, duration);
    interface::output::clustering::writeOutput(outfile + "_Classic", output_classic, status);
    cluster_classic.free_output_object_memory(output_classic);



    /* create a Clustering object in order to perform the clustering */
    clustering::Clustering<uint8_t> cluster_LSH(cluster_config, data);
    /* perform the clustering */
    cluster_LSH.perform_clustering(data, "LSH", &duration);

    /* get the silhouette and print it */
    average_silhouette = cluster_LSH.compute_average_silhouette(data);

    /* create an Output object, build it and use it to log the results to the outfile */
    interface::output::clustering::ClusteringOutput output_LSH;
    cluster_input.algorithm = "LSH";
    cluster_LSH.build_output(output_LSH, data, cluster_input, duration);
    interface::output::clustering::writeOutput(outfile + "_LSH", output_LSH, status);
    cluster_LSH.free_output_object_memory(output_LSH);


    /* create a Clustering object in order to perform the clustering */
    clustering::Clustering<uint8_t> cluster_HC(cluster_config, data);
    /* perform the clustering */
    cluster_HC.perform_clustering(data, "Hypercube", &duration);

    /* get the silhouette and print it */
    average_silhouette = cluster_HC.compute_average_silhouette(data);

    /* create an Output object, build it and use it to log the results to the outfile */
    interface::output::clustering::ClusteringOutput output_HC;
    cluster_input.algorithm = "Hypercube";
    cluster_HC.build_output(output_HC, data, cluster_input, duration);
    interface::output::clustering::writeOutput(outfile + "_Hypercube", output_HC, status);
    cluster_HC.free_output_object_memory(output_HC);


    K++;
  }




  /* free up the allocated space and return */
  interface::freeDataset(dataset);
  // cluster.free_output_object_memory(output);

  std::cout << "Finished at: " << currentDateTime() << std::endl;

  return 0;
}
