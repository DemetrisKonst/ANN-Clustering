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
      typedef struct clustering_input
      {
        bool complete = false;
        std::string algorithm = "";
      } clustering_input;

      /* struct used to define the clustering input from the configuration file */
      typedef struct clustering_config
      {
        uint16_t clusters_K = 10;
        uint8_t LSH_L = 3;
        uint8_t LSH_k = 4;
        uint32_t HC_M = 10;
        uint8_t HC_k = 3;
        uint16_t HC_probes = 2;
      } clustering_config;

      /* function to parse command line parameters */
      int clusteringParseInput(const int& argc, const char* argv[], clustering_input& input, IOCFiles& files, ExitCode& status);

      /* function to parse configuration file */
      int clusteringParseConfigFile(const std::string& filename, clustering_config& config, ExitCode& status);
    }
  }
}


#endif
