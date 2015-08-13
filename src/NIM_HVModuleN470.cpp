#include "NIM_HVModuleN470.h"

namespace NIM
{
  HVModuleN470::HVModuleN470(uint16_t addr, VME::CAENETControllerV288& cont) :
    fController(cont), fAddress(addr)
  {;}

  std::string
  HVModuleN470::GetModuleId() const
  {
    std::string word = "";
    std::vector<unsigned short> out;
    try {
      ReadRegister(kN470GeneralInfo, &out, 16);
      for (std::vector<unsigned short>::iterator it=out.begin(); it!=out.end(); it++) {
        word += static_cast<char>((*it)&0xff);
      }
    } catch (Exception& e) {
      e.Dump();
    }
    return word;
  }

  HVModuleN470Values
  HVModuleN470::ReadMonitoringValues() const
  {
    std::vector<unsigned short> out;
    try { ReadRegister(kN470MonStatus, &out, 16); } catch (Exception& e) {
      e.Dump();
    }
    HVModuleN470Values vals(out);
    //vals.Dump();
    return vals;
  }

  HVModuleN470ChannelValues
  HVModuleN470::ReadChannelValues(unsigned short ch_id) const
  {
    if (ch_id<0 or ch_id>=NUM_CHANNELS) throw Exception(__PRETTY_FUNCTION__, "Invalid channel id", JustWarning);
    std::vector<unsigned short> out;
    const HVModuleN470Opcodes opc = static_cast<HVModuleN470Opcodes>((unsigned short)(kN470OperationalParams&0xff)+(ch_id<<8));
    try { ReadRegister(opc, &out, 11); } catch (Exception& e) {
      e.Dump();
    }
    HVModuleN470ChannelValues vals(ch_id, out);
    //vals.Dump();
    return vals;
  }

  void
  HVModuleN470::SetChannelV0(unsigned short ch_id, unsigned short v0) const
  {
    if (ch_id<0 or ch_id>=NUM_CHANNELS) throw Exception(__PRETTY_FUNCTION__, "Invalid channel id", JustWarning);
    const HVModuleN470Opcodes opc = static_cast<HVModuleN470Opcodes>((unsigned short)(kN470V0Value&0xff)+(ch_id<<8));
    try { WriteRegister(opc, v0); } catch (Exception& e) {
      e.Dump();
      std::ostringstream os; os << "Failed to set V0 to " << v0 << " V on channel " << ch_id;
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }
  }

  void
  HVModuleN470::SetChannelI0(unsigned short ch_id, unsigned short i0) const
  {
    if (ch_id<0 or ch_id>=NUM_CHANNELS) throw Exception(__PRETTY_FUNCTION__, "Invalid channel id", JustWarning);
    const HVModuleN470Opcodes opc = static_cast<HVModuleN470Opcodes>((unsigned short)(kN470I0Value&0xff)+(ch_id<<8));
    try { WriteRegister(opc, i0); } catch (Exception& e) {
      e.Dump();
      std::ostringstream os; os << "Failed to set I0 to " << i0 << " uA on channel " << ch_id;
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }
  }

  void
  HVModuleN470::SetChannelV1(unsigned short ch_id, unsigned short v1) const
  {
    if (ch_id<0 or ch_id>=NUM_CHANNELS) throw Exception(__PRETTY_FUNCTION__, "Invalid channel id", JustWarning);
    const HVModuleN470Opcodes opc = static_cast<HVModuleN470Opcodes>((unsigned short)(kN470V1Value&0xff)+(ch_id<<8));
    try { WriteRegister(opc, v1); } catch (Exception& e) {
      e.Dump();
      std::ostringstream os; os << "Failed to set V1 to " << v1 << " V on channel " << ch_id;
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }
  }

  void
  HVModuleN470::SetChannelI1(unsigned short ch_id, unsigned short i1) const
  {
    if (ch_id<0 or ch_id>=NUM_CHANNELS) throw Exception(__PRETTY_FUNCTION__, "Invalid channel id", JustWarning);
    const HVModuleN470Opcodes opc = static_cast<HVModuleN470Opcodes>((unsigned short)(kN470I1Value&0xff)+(ch_id<<8));
    try { WriteRegister(opc, i1); } catch (Exception& e) {
      e.Dump();
      std::ostringstream os; os << "Failed to set I1 to " << i1 << " uA on channel " << ch_id;
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }
  }

  void
  HVModuleN470::ReadRegister(const HVModuleN470Opcodes& reg, std::vector<uint16_t>* data, unsigned int num_words) const
  {
    try {
      fController << (uint16_t)MSTIDENT;
      fController << (uint16_t)fAddress;
      fController << (uint16_t)reg;
      fController.SendBuffer();
      *data = fController.FetchBuffer(num_words);
      if (data->size()!=num_words)
        throw Exception(__PRETTY_FUNCTION__, "Wrong word size retrieved", JustWarning);
    } catch (Exception& e) {
      e.Dump();
      std::ostringstream o;
      o << "Impossible to read register at 0x" << std::hex << reg << "\n\t"
        << "Base address: 0x" << std::hex << fAddress;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  HVModuleN470::WriteRegister(const HVModuleN470Opcodes& reg, const std::vector<uint16_t>& data) const
  {
    try {
      fController << (uint16_t)MSTIDENT;
      fController << (uint16_t)fAddress;
      fController << (uint16_t)reg;
      for (std::vector<uint16_t>::const_iterator it=data.begin(); it!=data.end(); it++) {
        fController << (uint16_t)(*it);
      }
      fController.SendBuffer();
      usleep(5000);
    } catch (Exception& e) {
      e.Dump();
      std::ostringstream o;
      o << "Impossible to read register at 0x" << std::hex << reg << "\n\t"
        << "Base address: 0x" << std::hex << fAddress;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }
  
  void
  HVModuleN470::WriteRegister(const HVModuleN470Opcodes& reg, const uint16_t& data) const
  {
    try {
      std::vector<uint16_t> v_data; v_data.push_back(data);
      WriteRegister(reg, v_data);
    } catch (Exception& e) { throw e; }
  }
}
