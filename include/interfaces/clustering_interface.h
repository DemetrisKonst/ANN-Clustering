#ifndef _CLUSTERING_INTERFACE
#define _CLUSTERING_INTERFACE

#include "interface.h"

/* namespace regarding interface utilities */
namespace interface
{
  /* namespace regarding command line parameters utilities */
  namespace input
  {
    /* namespace regarding the clustering algorithms */
    namespace clustering
    {
      /* struct used to define the clustering input from the command line */
      typedef struct ClusteringInput
      {
        bool complete = false;
        std::string algorithm = "";
      } ClusteringInput;

      /* struct used to define the clustering input from the configuration file */
      typedef struct ClusteringConfig
      {
        uint16_t clusters_K = 10;
        uint8_t LSH_L = 3;
        uint8_t LSH_k = 4;
        uint32_t HC_M = 10;
        uint8_t HC_k = 3;
        uint16_t HC_probes = 2;
      } ClusteringConfig;

      /* function to parse command line parameters */
      int ClusteringParseInput(const int& argc, const char* argv[], ClusteringInput& input, IOCFiles& files, ExitCode& status);

      /* function to parse configuration file */
      int ClusteringParseConfigFile(const std::string& filename, ClusteringConfig& config, ExitCode& status);
    }
  }

  /* namespace regarding output file utilities */
  namespace output
  {
    /* namespace regarding clustering */
    namespace clustering
    {
      /* struct used to group all the clustering output information */
      typedef struct ClusteringOutput
      {
        uint16_t K = 10;
        uint16_t d = 0;
        std::string method = "Classic";
        std::vector<int> cluster_sizes;
        std::vector<uint8_t*> centroids;
        double clustering_time = 0.0;
        double* cluster_silhouettes = NULL;
        double total_silhouette = 0.0;
        bool complete = false;
        std::vector<Item<uint8_t>*>** items;
      } ClusteringOutput;

      /* function that writes the desired output to the outfile */
      int writeOutput(const std::string& outfile_name, const ClusteringOutput& output, ExitCode& status);
    }
  }
}


#endif
