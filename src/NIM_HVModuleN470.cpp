#include "NIM_HVModuleN470.h"

namespace NIM
{
  HVModuleN470::HVModuleN470(uint16_t addr, VME::CAENETControllerV288& cont) :
    fController(cont), fAddress(addr)
  {;}

  unsigned short
  HVModuleN470::GetModuleId()
  {
    unsigned short word = 0x0;
    std::vector<unsigned short> out;
    try {
      ReadRegister(kN470GeneralInfo, &out, 16);
      for (std::vector<unsigned short>::iterator it=out.begin(); it!=out.end(); it++) {
        std::cout << "word -> " << *it << std::endl;
      }
    } catch (Exception& e) {
      e.Dump();
    }
    return word;
  }

  void
  HVModuleN470::ReadRegister(const HVModuleN470Opcodes& reg, std::vector<uint16_t>* data, unsigned int num_words)
  {
    try {
      fController << (uint16_t)MSTIDENT;
      fController << (uint16_t)fAddress;
      fController << (uint16_t)reg;
      fController.SendBuffer();
      *data = fController.FetchBuffer(num_words);
    } catch (Exception& e) {
      e.Dump();
      std::ostringstream o;
      o << "Impossible to read register at 0x" << std::hex << reg << "\n\t"
        << "Base address: 0x" << std::hex << fAddress;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

}
