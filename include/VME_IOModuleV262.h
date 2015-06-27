#ifndef VME_IOModuleV262_h
#define VME_IOModuleV262_h

#include "VME_GenericBoard.h"

namespace VME
{
  enum IOModuleV262Register {
    kECLLevelWrite = 0x04,
    kNIMLevelWrite = 0x06,
    kNIMPulseWrite = 0x08,
    kNIMPulseRead  = 0x0a,
    kIdentifier    = 0xfa,
    kBoardInfo0    = 0xfc,
    kBoardInfo1    = 0xfe
  };
  class IOModuleV262 : public GenericBoard<IOModuleV262Register,cvA24_U_DATA>
  {
    public:
      IOModuleV262(int32_t bhandle, uint32_t baseaddr);
      inline ~IOModuleV262() {;}

      unsigned short GetSerialNumber() const;
      unsigned short GetModuleVersion() const;
      unsigned short GetModuleType() const;
      unsigned short GetManufacturerId() const;
      unsigned short GetIdentifier() const;

    private:

  };
}

#endif
