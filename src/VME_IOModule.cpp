#include "VME_IOModule.h"

namespace VME
{
  IOModule::IOModule(int32_t bhandle, uint32_t baseaddr) :
    GenericBoard<IOModuleRegister,cvA24_U_DATA>(bhandle, baseaddr)
  {
    std::ostringstream os;
    os << "New I/O module added:" << "\n\t"
       << "  Identifier: 0x" << std::hex << GetIdentifier() << "\n\t"
       << "  Serial number: " << std::dec << GetSerialNumber() << "\n\t"
       << "  Version: " << std::dec << GetModuleVersion() << "\n\t"
       << "  Type: " << std::dec << GetModuleType() << "\n\t"
       << "  Manufacturer: 0x" << std::hex << GetManufacturerId();
    PrintInfo(os.str());

    uint16_t word = 0xff;
    try {
      WriteRegister(kNIMLevelWrite, word);
    } catch (Exception& e) { e.Dump(); }
  }

  unsigned short
  IOModule::GetSerialNumber() const
  {
    uint16_t word;
    try {
      ReadRegister(kBoardInfo1, &word);
      return static_cast<unsigned short>(word&0xfff);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  unsigned short
  IOModule::GetModuleVersion() const
  {
    uint16_t word;
    try {
      ReadRegister(kBoardInfo1, &word);
      return static_cast<unsigned short>((word>>12)&0xf);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  unsigned short
  IOModule::GetModuleType() const
  {
    uint16_t word;
    try {
      ReadRegister(kBoardInfo0, &word);
      return static_cast<unsigned short>(word&0x3ff);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }
  
  unsigned short
  IOModule::GetManufacturerId() const
  {
    uint16_t word;
    try {
      ReadRegister(kBoardInfo0, &word);
      return static_cast<unsigned short>((word>>10)&0x3f);
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }

  unsigned short
  IOModule::GetIdentifier() const
  {
    uint16_t word;
    try {
      ReadRegister(kIdentifier, &word);
      return word;
    } catch (Exception& e) { e.Dump(); }
    return 0;
  }
}
