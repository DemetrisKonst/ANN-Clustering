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
}


#endif
