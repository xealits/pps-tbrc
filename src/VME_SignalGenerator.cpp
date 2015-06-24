#include "VME_SignalGenerator.h"

namespace VME
{
  SignalGenerator::SignalGenerator(int32_t bhandle, uint32_t baseaddr) :
    fHandle(bhandle), fBaseAddr(baseaddr)
  {
  }

  unsigned short
  SignalGenerator::GetSerialNumber() const
  {
    uint16_t word;
    try {
      ReadRegister(kBoardInfo1, &word);
      return static_cast<unsigned short>(word&0xFFF);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  unsigned short
  SignalGenerator::GetModuleVersion() const
  {
    uint16_t word;
    try {
      ReadRegister(kBoardInfo1, &word);
      return static_cast<unsigned short>((word>>12)&0xF);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  unsigned short
  SignalGenerator::GetModuleType() const
  {
    uint16_t word;
    try {
      ReadRegister(kBoardInfo0, &word);
      return static_cast<unsigned short>(word&0x3FF);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }
  
  unsigned short
  SignalGenerator::GetManufacturerId() const
  {
    uint16_t word;
    try {
      ReadRegister(kBoardInfo0, &word);
      return static_cast<unsigned short>((word>>10)&0x3F);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  void
  SignalGenerator::WriteRegister(const ModuleRegister& reg, const uint16_t& data) const
  {
    uint32_t address = fBaseAddr+reg;
    uint16_t* fdata = new uint16_t; *fdata = data;
    if (CAENVME_WriteCycle(fHandle, address, fdata, cvA32_U_DATA, cvD16)!=cvSuccess) {
      std::ostringstream o; o << "Impossible to write register at 0x" << std::hex << reg;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  SignalGenerator::ReadRegister(const ModuleRegister& reg, uint16_t* data) const
  {
    uint32_t address = fBaseAddr+reg;
    if (CAENVME_ReadCycle(fHandle, address, data, cvA32_U_DATA, cvD16)!=cvSuccess) {
      std::ostringstream o; o << "Impossible to read register at 0x" << std::hex << reg;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  } 
}
