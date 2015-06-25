#ifndef VME_IOModule_h
#define VME_IOModule_h

#include "VME_GenericBoard.h"

namespace VME
{
  enum IOModuleRegister {
    kECLLevelWrite = 0x04,
    kNIMLevelWrite = 0x06,
    kNIMPulseWrite = 0x08,
    kNIMPulseRead  = 0x0a,
    kIdentifier    = 0xfa,
    kBoardInfo0    = 0xfc,
    kBoardInfo1    = 0xfe
  };
  class IOModule : public GenericBoard<IOModuleRegister,cvA24_U_DATA>
  {
    public:
      inline IOModule(int32_t bhandle, uint32_t baseaddr) :
        GenericBoard<IOModuleRegister,cvA24_U_DATA>(bhandle, baseaddr) {;}
      inline ~IOModule() {;}

      unsigned short GetSerialNumber() const;
      unsigned short GetModuleVersion() const;
      unsigned short GetModuleType() const;
      unsigned short GetManufacturerId() const;

    private:

  };
}

#endif
