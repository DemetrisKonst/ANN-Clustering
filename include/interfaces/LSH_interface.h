#ifndef __LSH_INTERFACE__
#define __LSH_INTERFACE__

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
    namespace LSH
    {
      /* struct used to define the LSH input (from the command line), the rest (epsilon, c) can be found in a configfile */
      typedef struct LSH_input
      {
        uint8_t k = 4;
        uint8_t L = 5;
        uint32_t N = 1;
        double R = 1.0;
      } LSH_input;

      /* function to parse command line parameters */
      int LSHParseInput(const int& argc, const char* argv[], LSH_input& input, IOFiles& files, ExitCode& status);
    }

  }



}


#endif