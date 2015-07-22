#include "VME_CFDV812.h"

namespace VME
{
  CFDV812::CFDV812(int32_t bhandle, uint32_t baseaddr) :
    GenericBoard<CFDV812Register,cvA24_U_DATA>(bhandle, baseaddr)
  {
  }

  unsigned short
  CFDV812::GetFixedCode() const
  {
    uint16_t word = 0x0;
    try {
      ReadRegister(kV812FixedCode, &word);
    } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve the fixed code", JustWarning);
    }
    return static_cast<unsigned int>(word);
  }

  unsigned short
  CFDV812::GetManufacturerId() const
  {
    uint16_t word = 0x0;
    try {
      ReadRegister(kV812Info0, &word);
    } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve the manufacturer identifier", JustWarning);
    }
    return static_cast<unsigned int>((word>>10)&0x3f);
  }

  unsigned short
  CFDV812::GetModuleType() const
  {
    uint16_t word = 0x0;
    try {
      ReadRegister(kV812Info0, &word);
    } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve the module type", JustWarning);
    }
    return static_cast<unsigned int>(word&0x3ff);
  }

  unsigned short
  CFDV812::GetModuleVersion() const
  {
    uint16_t word = 0x0;
    try {
      ReadRegister(kV812Info1, &word);
    } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve the module version", JustWarning);
    }
    return static_cast<unsigned int>((word>>12)&0xf);
  }

  unsigned short
  CFDV812::GetSerialNumber() const
  {
    uint16_t word = 0x0;
    try {
      ReadRegister(kV812Info1, &word);
    } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve the serial number", JustWarning);
    }
    return static_cast<unsigned int>(word&0xfff);
  }

}
