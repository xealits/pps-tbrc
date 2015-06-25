#ifndef VME_FPGAUnitV1495_h
#define VME_FPGAUnitV1495_h

#include "VME_GenericBoard.h"

namespace VME
{
  enum FPGAUnitV1495Register {
    kV1495GeoAddress       = 0x8008,
    kV1495ModuleReset      = 0x800a,
    kV1495FWRevision       = 0x800c,
    kV1495ConfigurationROM = 0x8100,
    kV1495OUI2             = 0x8124,
    kV1495OUI1             = 0x8128,
    kV1495OUI0             = 0x812c,
    kV1495Board2           = 0x8134,
    kV1495Board1           = 0x8138,
    kV1495Board0           = 0x813c,
    kV1495HWRevision3      = 0x8140,
    kV1495HWRevision2      = 0x8144,
    kV1495HWRevision1      = 0x8148,
    kV1495HWRevision0      = 0x814c,
    kV1495SerNum0          = 0x8180,
    kV1495SerNum1          = 0x8184
  };
  class FPGAUnitV1495 : public GenericBoard<FPGAUnitV1495Register,cvA32_U_DATA>
  {
    public:
      FPGAUnitV1495(int32_t bhandle, uint32_t baseaddr);
      inline ~FPGAUnitV1495() {;}

      unsigned short GetFirmwareRevision() const;
      unsigned int GetHardwareRevision() const;
      unsigned short GetSerialNumber() const;
      unsigned short GetGeoAddress() const;
      void CheckBoardVersion() const;

    private:

  };
}

#endif
