#include "VME_SignalGenerator.h"

namespace VME
{
  SignalGenerator::SignalGenerator(int32_t bhandle, uint32_t baseaddr) :
    fHandle(bhandle), fBaseAddr(baseaddr)
  {
  }
  
}
