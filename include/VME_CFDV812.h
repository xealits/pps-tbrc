#ifndef VME_CFDV812_h
#define VME_CFDV812_h

#include "VME_GenericBoard.h"

namespace VME
{
  enum CFDV812Register
  {
    kV812ThresholdChannel0 = 0x00,
    kV812OutputWidthGroup0 = 0x40,
    kV812OutputWidthGroup1 = 0x42,
    kV812DeadTimeGroup0    = 0x44,
    kV812DeadTimeGroup1    = 0x46,
    kV812MajorityThreshold = 0x48,
    kV812PatternOfInhibit  = 0x4a,
    kV812TestPulse         = 0x4c,
    kV812FixedCode         = 0xfa,
    kV812Info0             = 0xfc,
    kV812Info1             = 0xfe
  };
  class CFDV812 : public GenericBoard<CFDV812Register,cvA24_U_DATA>
  {
    public:
      CFDV812(int32_t bhandle, uint32_t baseaddr);
      inline ~CFDV812() {;}

      unsigned short GetFixedCode() const;
      unsigned short GetManufacturerId() const;
      unsigned short GetModuleType() const;
      unsigned short GetModuleVersion() const;
      unsigned short GetSerialNumber() const;

    private:
  };
}

#endif
