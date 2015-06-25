#include "VME_FPGAUnitV1495.h"

namespace VME
{
  FPGAUnitV1495::FPGAUnitV1495(int32_t bhandle, uint32_t baseaddr) :
    GenericBoard<FPGAUnitV1495Register,cvA32_U_DATA>(bhandle, baseaddr)
  {
    try {
      CheckOUI();
    } catch (Exception& e) { e.Dump(); }

    unsigned short fwrev = GetFirmwareRevision();
    std::ostringstream os;
    os << "New FPGA unit added:" << "\n\t"
     /*<< "Identifier: 0x" << std::hex << GetIdentifier() << "\n\t"
       << "Serial number: " << std::dec << GetSerialNumber() << "\n\t"
       << "Version: " << std::dec << GetModuleVersion() << "\n\t"
       << "Type: " << std::dec << GetModuleType() << "\n\t"
       << "Manufacturer: 0x" << std::hex << GetManufacturerId();*/
       << "Firmware revision: v" << ((fwrev>>8)&0xff) << "." << (fwrev&0xff);
    PrintInfo(os.str());
  }

  unsigned short
  FPGAUnitV1495::GetFirmwareRevision() const
  {
    uint16_t word;
    try {
      ReadRegister(kV1495FWRevision, &word);
      return static_cast<unsigned short>(word&0xffff);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  void
  FPGAUnitV1495::CheckOUI() const
  {
    uint64_t oui;
    uint16_t oui0, oui1, oui2;
    uint16_t word;
    try {
      ReadRegister(kV1495OUI0, &word); oui0 = word; oui  =  (word&0xFF);
      ReadRegister(kV1495OUI1, &word); oui1 = word; oui |= ((word&0xFF)<<8);
      ReadRegister(kV1495OUI2, &word); oui2 = word; oui |= ((word&0xFF)<<16);
    } catch (Exception& e) { throw e; }

    if (oui0!=0xe6 or oui1!=0x40 or oui2!=0x00) {
      std::ostringstream os;
      os << "Wrong OUI version: " << std::hex
         << oui0 << "/" << oui1 << "/" << oui2;
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }
  }
}
