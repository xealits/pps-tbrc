#ifndef VME_SignalGenerator_h
#define VME_SignalGenerator_h

#include "VME_GenericBoard.h"

namespace VME
{
  enum SignalGeneratorRegister {
    kECLLevelWrite = 0x04,
    kNIMLevelWrite = 0x06,
    kNIMPulseWrite = 0x08,
    kNIMPulseRead  = 0x0A,
    kIdentifier    = 0xFA,
    kBoardInfo0    = 0xFC,
    kBoardInfo1    = 0xFE
  };
  class SignalGenerator : public GenericBoard<SignalGeneratorRegister>
  {
    public:
      inline SignalGenerator(int32_t bhandle, uint32_t baseaddr) :
        GenericBoard<SignalGeneratorRegister>(bhandle, baseaddr) {;}
      inline ~SignalGenerator() {;}

      unsigned short GetSerialNumber() const;
      unsigned short GetModuleVersion() const;
      unsigned short GetModuleType() const;
      unsigned short GetManufacturerId() const;

    private:

  };
}

#endif
