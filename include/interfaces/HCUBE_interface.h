#ifndef _HCUBE_INTERFACE
#define _HCUBE_INTERFACE

#include <cstdlib>
#include <cstdint>

#include "interface.h"

/* namespace regarding interface utilities */
namespace interface
{

  /* namespace regarding command line parameters utilities */
  namespace input
  {

    /* namespace regarding LSH */
    namespace HCUBE
    {
      /* struct used to define the LSH input (from the command line), the rest (epsilon, c) can be found in a configfile */
      typedef struct HCUBE_input
      {
        uint8_t k = 4;
        uint32_t M = 10;
        uint16_t probes = 2;
        uint32_t N = 1;
        double R = 1.0;
      } LSH_input;

      /* function to parse command line parameters */
      int HCUBEParseInput(const int& argc, const char* argv[], HCUBE_input& input, IOFiles& files, ExitCode& status);
    }

  }



}


#endif
