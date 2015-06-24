#include "VME_IOModule.h"

namespace VME
{
  unsigned short
  IOModule::GetSerialNumber() const
  {
    uint16_t word;
    try {
      ReadRegister(kBoardInfo1, &word);
      return static_cast<unsigned short>(word&0xFFF);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  unsigned short
  IOModule::GetModuleVersion() const
  {
    uint16_t word;
    try {
      ReadRegister(kBoardInfo1, &word);
      return static_cast<unsigned short>((word>>12)&0xF);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  unsigned short
  IOModule::GetModuleType() const
  {
    uint16_t word;
    try {
      ReadRegister(kBoardInfo0, &word);
      return static_cast<unsigned short>(word&0x3FF);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }
  
  unsigned short
  IOModule::GetManufacturerId() const
  {
    uint16_t word;
    try {
      ReadRegister(kBoardInfo0, &word);
      return static_cast<unsigned short>((word>>10)&0x3F);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }
}
