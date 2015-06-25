#ifndef VME_FPGAUnitV1495_h
#define VME_FPGAUnitV1495_h

#include "VME_GenericBoard.h"

namespace VME
{
  enum FPGAUnitV1495Register {
    kV1495ModuleReset      = 0x800a,
    kV1495FWRevision       = 0x800c,
    kV1495ConfigurationROM = 0x8100,
    kV1495OUI2             = 0x8124,
    kV1495OUI1             = 0x8128,
    kV1495OUI0             = 0x812c
  };
  class FPGAUnitV1495 : public GenericBoard<FPGAUnitV1495Register,cvA32_U_DATA>
  {
    public:
      FPGAUnitV1495(int32_t bhandle, uint32_t baseaddr);
      inline ~FPGAUnitV1495() {;}

      unsigned short GetFirmwareRevision() const;
      void CheckOUI() const;

    private:

  };
}

#endif
