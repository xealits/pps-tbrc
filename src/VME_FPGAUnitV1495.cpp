#include "VME_FPGAUnitV1495.h"

namespace VME
{
  FPGAUnitV1495::FPGAUnitV1495(int32_t bhandle, uint32_t baseaddr) :
    GenericBoard<FPGAUnitV1495Register,cvA32_U_DATA>(bhandle, baseaddr)
  {
    try {
      CheckBoardVersion();
    } catch (Exception& e) { e.Dump(); }

    unsigned short cfwrev = GetCAENFirmwareRevision(), ufwrev = GetUserFirmwareRevision();
    unsigned int hwrev = GetHardwareRevision();
    std::ostringstream os;
    os << "New FPGA unit added at base address 0x" << std::hex << fBaseAddr << "\n\t"
     /*<< "Identifier: 0x" << std::hex << GetIdentifier() << "\n\t"
       << "Version: " << std::dec << GetModuleVersion() << "\n\t"
       << "Type: " << std::dec << GetModuleType() << "\n\t"
       << "Manufacturer: 0x" << std::hex << GetManufacturerId();*/
       << "Serial number: " << std::dec << GetSerialNumber() << "\n\t"
       << "Hardware revision: " << std::dec
                                << ((hwrev>>24)&0xff) << "."
                                << ((hwrev>>16)&0xff) << "."
                                << ((hwrev>>8)&0xff) << "."
                                << (hwrev&0xff) << "\n\t"
       << "CAEN Firmware revision: " << std::dec << ((cfwrev>>8)&0xff) << "." << (cfwrev&0xff) << "\n\t"
       << "User Firmware revision: " << std::dec << ((ufwrev>>8)&0xff) << "." << (ufwrev&0xff) << "\n\t"
       << "Geo address: 0x" << std::hex << GetGeoAddress();
    PrintInfo(os.str());

    //uint32_t word, word_read;
    /*unsigned short bits;
    //for (int i=0; i<1000; i++) {
    int i = 0;
    while (true) {
      if (i%2==0) bits = kReset|kTrigger|kClear;
      else        bits = 0x0;
      SetTDCBits(bits); std::cout << "word: 0x" << std::hex << GetTDCBits() << std::endl;
      //WriteRegister((FPGAUnitV1495Register)0x1018, word);
      //ReadRegister((FPGAUnitV1495Register)0x1018, &word_read);
      i++;
      usleep(1000);
    }*/
    //ReadRegister((FPGAUnitV1495Register)0x1018, &word);

    //uint32_t word = 0x3;
    //WriteRegister((FPGAUnitV1495Register)0x1018, word);
  }

  unsigned short
  FPGAUnitV1495::GetCAENFirmwareRevision() const
  {
    uint16_t word;
    try {
      ReadRegister(kV1495FWRevision, &word);
      return static_cast<unsigned short>(word&0xffff);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  unsigned short
  FPGAUnitV1495::GetUserFirmwareRevision() const
  {
    uint16_t word;
    try {
      ReadRegister(kV1495UserFWRevision, &word);
      return static_cast<unsigned short>(word&0xffff);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  unsigned int
  FPGAUnitV1495::GetHardwareRevision() const
  {
    uint16_t word;
    uint32_t hwrev;
    try {
      ReadRegister(kV1495HWRevision0, &word); hwrev  =  (word&0xff);
      ReadRegister(kV1495HWRevision1, &word); hwrev |= ((word&0xff)<<8);
      ReadRegister(kV1495HWRevision2, &word); hwrev |= ((word&0xff)<<16);
      ReadRegister(kV1495HWRevision3, &word); hwrev |= ((word&0xff)<<24);
      return hwrev;
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  unsigned short
  FPGAUnitV1495::GetSerialNumber() const
  {
    uint16_t word;
    uint16_t sernum;
    try {
      ReadRegister(kV1495SerNum1, &word); sernum  =  (word&0xff);
      ReadRegister(kV1495SerNum0, &word); sernum |= ((word&0xff)<<8);
      return sernum;
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  unsigned short
  FPGAUnitV1495::GetGeoAddress() const
  {
    uint16_t word;
    try {
      ReadRegister(kV1495GeoAddress, &word);
      return (word);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  void
  FPGAUnitV1495::CheckBoardVersion() const
  {
    uint16_t word;
    uint16_t oui0, oui1, oui2;
    uint16_t board0, board1, board2;

    // OUI
    try {
      ReadRegister(kV1495OUI0, &word); oui0 = word&0xff;
      ReadRegister(kV1495OUI1, &word); oui1 = word&0xff;
      ReadRegister(kV1495OUI2, &word); oui2 = word&0xff;
    } catch (Exception& e) { throw e; }
    if (oui0!=0xe6 or oui1!=0x40 or oui2!=0x00) {
      std::ostringstream os;
      os << "Wrong OUI version: " << std::hex
         << oui0 << "/" << oui1 << "/" << oui2;
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }

    // Board version
    try {
      ReadRegister(kV1495Board0, &word); board0 = word&0xff;
      ReadRegister(kV1495Board1, &word); board1 = word&0xff;
      ReadRegister(kV1495Board2, &word); board2 = word&0xff;
    } catch (Exception& e) { throw e; }
    if (board0!=0xd7 or board1!=0x05 or board2!=0x00) {
      std::ostringstream os;
      os << "Wrong board version: " << std::hex
         << board0 << "/" << board1 << "/" << board2;
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }
  }

  unsigned short
  FPGAUnitV1495::GetTDCBits() const
  {
    uint32_t word;
    try {
      ReadRegister(kV1495TDCBoardInterface, &word);
      return static_cast<unsigned short>(word&0x7);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  void
  FPGAUnitV1495::SetTDCBits(unsigned short bits) const
  {
    uint32_t word = (bits&0x7);
    try {
      if (bits==GetTDCBits()) return;
      WriteRegister(kV1495TDCBoardInterface, word);
    } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to set TDC bits", JustWarning);
    }
  }

  void
  FPGAUnitV1495::PulseTDCBits(unsigned short bits, unsigned int time_us) const
  {
    // FIXME need to check what is the previous bits status before any pulse -> exception?
    try {
      SetTDCBits(bits);
      usleep(time_us); // we let it high for a given time (in ns)
      SetTDCBits(0x0);
    } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to send a pulse to TDC bits", JustWarning);
    }
  }

  FPGAUnitV1495Control
  FPGAUnitV1495::GetControl() const
  {
    uint32_t word;
    try { ReadRegister(kV1495Control, &word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve the control word from FW", JustWarning);
    }
    return FPGAUnitV1495Control(word);
  }

  void
  FPGAUnitV1495::SetControl(const FPGAUnitV1495Control& control) const
  {
    try { WriteRegister(kV1495Control, control.GetWord()); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to set the control word to FW", JustWarning);
    }
  }

  void
  FPGAUnitV1495::SetClockPeriod(uint32_t period) const
  {
    try { WriteRegister(kV1495ClockSettings, period); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to set internal clock's period", JustWarning);
    }
  }

  uint32_t
  FPGAUnitV1495::GetClockPeriod() const
  {
    uint32_t word;
    try { ReadRegister(kV1495ClockSettings, &word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve internal clock's period", JustWarning);
      return 0;
    }
    return word;
  }

  void
  FPGAUnitV1495::SetTriggerPeriod(uint32_t period) const
  {
    try { WriteRegister(kV1495TriggerSettings, period); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to set internal trigger's period", JustWarning);
    }
  }

  uint32_t
  FPGAUnitV1495::GetTriggerPeriod() const
  {
    uint32_t word;
    try { ReadRegister(kV1495TriggerSettings, &word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve internal trigger's period", JustWarning);
      return 0;
    }
    return word;
  }
}
