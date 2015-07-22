#ifndef VME_CFDV812_h
#define VME_CFDV812_h

#include "VME_GenericBoard.h"
#include <map>

#define num_cfd_channels 16

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

      void CheckConfiguration() const;

      unsigned short GetFixedCode() const;
      unsigned short GetManufacturerId() const;
      unsigned short GetModuleType() const;
      unsigned short GetModuleVersion() const;
      unsigned short GetSerialNumber() const;

      void SetOutputWidth(unsigned short group_id, unsigned short value) const;
      /// Set the threshold for one single channel, in units of 1 mV
      void SetThreshold(unsigned short channel_id, unsigned short value) const;

    private:
      float OutputWidthCalculator(unsigned short value) const;
  };
}

#endif
