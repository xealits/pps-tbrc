#ifndef VME_SignalGenerator_h
#define VME_SignalGenerator_h

#include <stdint.h>

namespace VME
{
  class SignalGenerator
  {
    public:
      SignalGenerator(int32_t bhandle, uint32_t baseaddr);
      inline ~SignalGenerator() {;}
    private:
      int32_t fHandle;
      uint32_t fBaseAddr;
  };
}

#endif
