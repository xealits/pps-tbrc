#ifndef VME_IOModule_h
#define VME_IOModule_h

#include "VME_GenericBoard.h"

namespace VME
{
  enum IOModuleRegister {
    kECLLevelWrite = 0x04,
    kNIMLevelWrite = 0x06,
    kNIMPulseWrite = 0x08,
    kNIMPulseRead  = 0x0A,
    kIdentifier    = 0xFA,
    kBoardInfo0    = 0xFC,
    kBoardInfo1    = 0xFE
  };
  class IOModule : public GenericBoard<IOModuleRegister>
  {
    public:
      inline IOModule(int32_t bhandle, uint32_t baseaddr) :
        GenericBoard<IOModuleRegister>(bhandle, baseaddr) {;}
      inline ~IOModule() {;}

      unsigned short GetSerialNumber() const;
      unsigned short GetModuleVersion() const;
      unsigned short GetModuleType() const;
      unsigned short GetManufacturerId() const;

    private:

  };
}

#endif
