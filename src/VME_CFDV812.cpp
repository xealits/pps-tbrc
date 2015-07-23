#include "VME_CFDV812.h"

namespace VME
{
  CFDV812::CFDV812(int32_t bhandle, uint32_t baseaddr) :
    GenericBoard<CFDV812Register,cvA24_U_DATA>(bhandle, baseaddr)
  {
    try {
      CheckConfiguration();
    } catch (Exception& e) {
      e.Dump();
    }
  }

  void
  CFDV812::CheckConfiguration() const
  {
    try {
      bool success = true;
      success |= (GetManufacturerId()==0x2);
      success |= (GetModuleType()==0x51);
      if (!success) {
        std::ostringstream os;
        os << "Wrong configuration retrieved from CFD with base address 0x" << std::hex << fBaseAddr;
        throw Exception(__PRETTY_FUNCTION__, os.str(), Fatal);
      }
    } catch (Exception& e) {
      e.Dump();
      std::ostringstream os; os << "Failed to retrieve configuration from CFD with base address 0x" << std::hex << fBaseAddr;
      throw Exception(__PRETTY_FUNCTION__, os.str(), Fatal);
    }
  }

  unsigned short
  CFDV812::GetFixedCode() const
  {
    uint16_t word = 0x0;
    try { ReadRegister(kV812FixedCode, &word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve the fixed code", JustWarning);
    }
    return static_cast<unsigned int>(word);
  }

  unsigned short
  CFDV812::GetManufacturerId() const
  {
    uint16_t word = 0x0;
    try { ReadRegister(kV812Info0, &word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve the manufacturer identifier", JustWarning);
    }
    return static_cast<unsigned int>((word>>10)&0x3f);
  }

  unsigned short
  CFDV812::GetModuleType() const
  {
    uint16_t word = 0x0;
    try { ReadRegister(kV812Info0, &word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve the module type", JustWarning);
    }
    return static_cast<unsigned int>(word&0x3ff);
  }

  unsigned short
  CFDV812::GetModuleVersion() const
  {
    uint16_t word = 0x0;
    try { ReadRegister(kV812Info1, &word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve the module version", JustWarning);
    }
    return static_cast<unsigned int>((word>>12)&0xf);
  }

  unsigned short
  CFDV812::GetSerialNumber() const
  {
    uint16_t word = 0x0;
    try { ReadRegister(kV812Info1, &word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve the serial number", JustWarning);
    }
    return static_cast<unsigned int>(word&0xfff);
  }

  void
  CFDV812::SetPOI(unsigned short poi) const
  {
    try { WriteRegister(kV812PatternOfInhibit, poi); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to set the pattern of inhibit", JustWarning);
    }
  }

  void
  CFDV812::SetThreshold(unsigned short channel_id, unsigned short value) const
  {
    if (channel_id<0 or channel_id>num_cfd_channels) return;
    try {
      CFDV812Register reg = static_cast<CFDV812Register>(kV812ThresholdChannel0+channel_id*0x2);
      WriteRegister(reg, value);
    } catch (Exception& e) {
      e.Dump();
      std::ostringstream os;
      os << "Failed to set the threshold for channel " << channel_id;
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }
  }

  void
  CFDV812::SetOutputWidth(unsigned short group_id, unsigned short value) const
  {
    uint16_t word = static_cast<uint16_t>(value&0xffff);
    CFDV812Register reg;
    if (group_id==0)      reg = kV812OutputWidthGroup0;
    else if (group_id==1) reg = kV812OutputWidthGroup1;
    else throw Exception(__PRETTY_FUNCTION__, "Requested to change the output width of an unrecognized group identifier", JustWarning);
    try {
      WriteRegister(reg, word);
    } catch (Exception& e) {
      e.Dump();
      std::ostringstream os;
      os << "Failed to set output width for group " << group_id
         << " to value " << value << " (~" << OutputWidthCalculator(value) << " ns)";
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }
    std::ostringstream os;
    os << "Output width from group " << group_id << " changed to ~" << OutputWidthCalculator(value) << " ns";
    PrintInfo(os.str());
  }
 
  void
  CFDV812::SetDeadTime(unsigned short group_id, unsigned short value) const
  {
    uint16_t word = static_cast<uint16_t>(value&0xffff);
    CFDV812Register reg;
    if (group_id==0)      reg = kV812DeadTimeGroup0;
    else if (group_id==1) reg = kV812DeadTimeGroup1;
    else throw Exception(__PRETTY_FUNCTION__, "Requested to change the dead time of an unrecognized group identifier", JustWarning);
    try {
      WriteRegister(reg, word);
    } catch (Exception& e) {
      e.Dump();
      std::ostringstream os;
      os << "Failed to set dead time for group " << group_id
         << " to value " << value << " (~" << DeadTimeCalculator(value) << " ns)";
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }
    std::ostringstream os;
    os << "Dead time from group " << group_id << " changed to ~" << DeadTimeCalculator(value) << " ns";
    PrintInfo(os.str());

  }
 
  float
  CFDV812::OutputWidthCalculator(unsigned short value) const
  {
    if (value>255 or value<0) return -1.;
    std::map<unsigned short,float> lookup_table;
    lookup_table[  0] = 11.32; lookup_table[ 15] = 12.34; lookup_table[ 30] = 13.47;
    lookup_table[ 45] = 14.75; lookup_table[ 60] = 16.07; lookup_table[ 75] = 17.51;
    lookup_table[ 90] = 19.03; lookup_table[105] = 21.29; lookup_table[120] = 23.69;
    lookup_table[135] = 26.71; lookup_table[150] = 30.61; lookup_table[165] = 35.20;
    lookup_table[180] = 41.83; lookup_table[195] = 51.02; lookup_table[210] = 64.53;
    lookup_table[225] = 87.47; lookup_table[240] =130.70; lookup_table[255] =240.70;
    std::map<unsigned short,float>::iterator it;
    if ((it=lookup_table.find(value))!=lookup_table.end()) return it->second;
    else {
      unsigned short last_id = 0; float last_value = -1.;
      for (it=lookup_table.begin(); it!=lookup_table.end(); it++) {
        if (value>it->first) { last_id = it->first; last_value = it->second; }
        else return (last_value+(it->second-last_value)/(value-last_id));
      }
    }
    return -1.;
  }

  float
  CFDV812::DeadTimeCalculator(unsigned short value) const
  {
    if (value>255 or value<0) return -1.;
    return (150.+(2000.-150.)/255*value);
  }
}
