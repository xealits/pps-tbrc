#include "VME_FPGAUnitV1495.h"

namespace VME
{
  FPGAUnitV1495::FPGAUnitV1495(int32_t bhandle, uint32_t baseaddr) :
    GenericBoard<FPGAUnitV1495Register,cvA32_U_DATA>(bhandle, baseaddr)
  {
    /*uint32_t registers[] = {0x1018, 0x101c, 0x1020, 0x1024, 0x1028};
    uint32_t word, word_in = 0x42;
    for (unsigned int i=0; i<sizeof(registers)/sizeof(registers[0]); i++) {
      try {
        WriteRegister((FPGAUnitV1495Register)registers[i], word_in);
        ReadRegister((FPGAUnitV1495Register)registers[i], &word);
        std::cout << "word 0x" << std::hex << registers[i] << " = 0x" << word << std::endl;
      } catch (Exception& e) { e.Dump(); }
    }
    exit(0);*/
    try {
      CheckBoardVersion();
      ClearOutputPulser();
    } catch (Exception& e) { e.Dump(); }

    unsigned short cfwrev = GetCAENFirmwareRevision();
    unsigned int hwrev = GetHardwareRevision();
    std::ostringstream os;
    os << "New FPGA unit added at base address 0x" << std::hex << fBaseAddr << "\n\t"
       << "Serial number: " << std::dec << GetSerialNumber() << "\n\t"
       << "Hardware revision: " << std::dec
                                << ((hwrev>>24)&0xff) << "."
                                << ((hwrev>>16)&0xff) << "."
                                << ((hwrev>>8)&0xff) << "."
                                << (hwrev&0xff) << "\n\t"
       << "CAEN Firmware revision: " << std::dec << ((cfwrev>>8)&0xff) << "." << (cfwrev&0xff) << "\n\t"
       << "Geo address: 0x" << std::hex << GetGeoAddress();
    PrintInfo(os.str());

    /*uint32_t word;
    try {
      //ReadRegister((FPGAUnitV1495Register)0x1020, &word);
      ReadRegister(kV1495Control, &word);
      std::cout << word << std::endl;
    } catch (Exception& e) { e.Dump(); }*/
    GetControl();
  }
 
  void
  FPGAUnitV1495::DumpFWInformation() const
  {
    unsigned short ufwrev = GetUserFirmwareRevision();
    FPGAUnitV1495Control control = GetControl();

    std::ostringstream os;
    os << "User Firmware information" << "\n\t"
       << "FW revision: " << std::dec << ((ufwrev>>8)&0xff) << "." << (ufwrev&0xff) << "\n\t"
       << "Clock source:   ";
    switch (control.GetClockSource()) {
      case FPGAUnitV1495Control::ExternalClock: os << "external"; break;
      case FPGAUnitV1495Control::InternalClock:
        os << "internal (period: " << (GetInternalClockPeriod()*25) << " ns)"; break;
      default: os << "invalid (" << control.GetClockSource() << ")"; break;
    }
    os << "\n\t"
       << "Trigger source: ";
    switch (control.GetTriggerSource()) {
      case FPGAUnitV1495Control::ExternalTrigger: os << "external"; break;
      case FPGAUnitV1495Control::InternalTrigger:
        os << "internal (period: " << (GetInternalTriggerPeriod()*25/1e6) << " ms)"; break;
      default: os << "invalid (" << control.GetTriggerSource() << ")"; break;
    }
    PrintInfo(os.str());
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
      usleep(time_us); // we let it high for a given time (in us)
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
  FPGAUnitV1495::ResetFPGA() const
  {
    uint32_t word = 0x1;
    try { WriteRegister(kV1495UserFPGAConfig, word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to reset the FPGA configuration", JustWarning);
    }
  }

  void
  FPGAUnitV1495::SetInternalClockPeriod(uint32_t period) const
  {
    //if (period<=0 or (period>1 and period%2!=0)) {
    if (period<=0) {
      std::ostringstream os; os << "Trying to set an invalid clock period (" << period << ")";
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }
    try { WriteRegister(kV1495ClockSettings, period); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to set internal clock's period", JustWarning);
    }
  }

  uint32_t
  FPGAUnitV1495::GetInternalClockPeriod() const
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
  FPGAUnitV1495::SetInternalTriggerPeriod(uint32_t period) const
  {
    if (period<=0) {
      std::ostringstream os; os << "Trying to set an invalid trigger period (" << period << ")";
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }
    try { WriteRegister(kV1495TriggerSettings, period); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to set internal trigger's period", JustWarning);
    }
  }

  uint32_t
  FPGAUnitV1495::GetInternalTriggerPeriod() const
  {
    uint32_t word;
    try { ReadRegister(kV1495TriggerSettings, &word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve internal trigger's period", JustWarning);
      return 0;
    }
    return word;
  }

  uint32_t
  FPGAUnitV1495::GetOutputPulser() const
  {
    uint32_t word;
    try { ReadRegister(kV1495OutputSettings, &word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve output pulser's word", JustWarning);
      return 0;
    }
    return word;
  }

  void
  FPGAUnitV1495::ClearOutputPulser() const
  {
    uint32_t word = 0x0;
    try { WriteRegister(kV1495OutputSettings, word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to clear output pulser's word", JustWarning);
    }
  }

  void
  FPGAUnitV1495::SetOutputPulser(unsigned short id, bool internal_trigger, bool enable) const
  {
    uint32_t word = GetOutputPulser();
    if (word>> id)     word -= (1<< id);
    if (word>>(id+16)) word -= (1<<(id+16));
    if (enable) {
      if (internal_trigger) word += (1<< id);
      else                  word += (1<<(id+16));
    }
    std::cout << word << std::endl;
    try { WriteRegister(kV1495OutputSettings, word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to set output pulser's word", JustWarning);
    }
  }
}
