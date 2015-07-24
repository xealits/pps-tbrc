#ifndef NIM_HVModuleN470_h
#define NIM_HVModuleN470_h

#include "VME_CAENETControllerV288.h"

namespace NIM
{
  class HVModuleN470
  {
    public:
      inline HVModuleN470(uint16_t addr, VME::CAENETControllerV288& cont) :
        fController(cont), fAddress(addr) {;}
      inline ~HVModuleN470() {;}

    private:
      VME::CAENETControllerV288 fController;
      uint16_t fAddress;
  };
}

#endif
