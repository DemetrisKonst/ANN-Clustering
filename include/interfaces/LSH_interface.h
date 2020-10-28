#ifndef _LSH_INTERFACE
#define _LSH_INTERFACE

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
      typedef struct LSHInput
      {
        uint8_t k = 4;
        uint8_t L = 5;
        uint32_t N = 1;
        double R = 10000.0;
      } LSHInput;

      /* function to parse command line parameters */
      int LSHParseInput(const int& argc, const char* argv[], LSHInput& input, IOFiles& files, ExitCode& status);
    }

  }



}


#endif
