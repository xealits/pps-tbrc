#include "VME_BridgeVx718.h"

namespace VME
{
  BridgeVx718::BridgeVx718(const char *device, BridgeType type)
  {
    int dev = atoi(device);
    CVBoardTypes tp;
    CVErrorCodes ret; 
    std::ostringstream o;
   
    char rel[20];
    CAENVME_SWRelease(rel);
    o.str("");
    o << "Initializing the VME bridge\n\t"
      << "CAEN library release: " << rel;
    PrintInfo(o.str());

    switch (type) {
      case CAEN_V1718: tp = cvV1718; break;
      case CAEN_V2718: tp = cvV2718; break;
      default:
        o.str("");
        o << "Invalid VME bridge type: " << type;
        throw Exception(__PRETTY_FUNCTION__, o.str(), Fatal);
    }
   
    ret = CAENVME_Init(tp, 0x0, dev, &fHandle);
    if (ret!=cvSuccess) {
      o.str("");
      o << "Error opening the VME bridge!\n\t"
        << "CAEN error: " << CAENVME_DecodeError(ret);
      throw Exception(__PRETTY_FUNCTION__, o.str(), Fatal);
    }

    ret = CAENVME_BoardFWRelease(fHandle, rel);
    if (ret!=cvSuccess) {
      o.str("");
      o << "Failed to retrieve the board FW release!\n\t"
        << "CAEN error: " << CAENVME_DecodeError(ret);
      throw Exception(__PRETTY_FUNCTION__, o.str(), Fatal);
    }
    o.str("");
    o << "Bridge firmware version: " << rel;
    PrintInfo(o.str());
    
    CheckConfiguration();
  }

  BridgeVx718::~BridgeVx718()
  {
    CAENVME_End(fHandle);
  }

  void
  BridgeVx718::CheckConfiguration() const
  {
    CVErrorCodes ret;
    CVDisplay config;
    std::ostringstream o;
    ret = CAENVME_ReadDisplay(fHandle, &config);
    if (ret!=cvSuccess) {
      o.str("");
      o << "Failed to retrieve configuration displayed on\n\t"
        << "module's front panel\n\t"
        << "CAEN error: " << CAENVME_DecodeError(ret);
      throw Exception(__PRETTY_FUNCTION__, o.str(), Fatal);
    }
  }

  // output := cvOutput[0,4] 
  void
  BridgeVx718::OutputConf(CVOutputSelect output) const
  {
    if (CAENVME_SetOutputConf(fHandle, output, cvDirect, cvActiveHigh, cvManualSW)!=cvSuccess) {
      std::ostringstream o;
      o << "Failed to configure output register #" << static_cast<int>(output);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  // output := cvOutput[0,4]
  void
  BridgeVx718::OutputOn(CVOutputSelect output) const
  {
    if (CAENVME_SetOutputRegister(fHandle, output)!=cvSuccess) {
      std::ostringstream o;
      o << "Failed to enable output register #" << static_cast<int>(output);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  BridgeVx718::OutputOff(CVOutputSelect output) const
  {
    if (CAENVME_ClearOutputRegister(fHandle, output)!=cvSuccess) {
      std::ostringstream o;
      o << "Failed to disable output register #" << static_cast<int>(output);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  // input := cvInput[0,1]
  void
  BridgeVx718::InputConf(CVInputSelect input) const
  {
    if (CAENVME_SetInputConf(fHandle, input, cvDirect, cvActiveHigh)!=cvSuccess) {
      std::ostringstream o;
      o << "Failed to configure input register #" << static_cast<int>(input);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  BridgeVx718::InputRead(CVInputSelect input) const
  {
    short unsigned int data;
    if (CAENVME_ReadRegister(fHandle, cvInputReg, &data)!=cvSuccess) {
      std::ostringstream o;
      o << "Failed to read data input register #" << static_cast<int>(input);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
    // decoding with CVInputRegisterBits
    std::cout << "Input line 0 status: " << ((data&cvIn0Bit) >> 0) << std::endl;
    std::cout << "Input line 1 status: " << ((data&cvIn1Bit) >> 1) << std::endl;
  }

  void
  BridgeVx718::StartPulser(double period, double width, unsigned char num_pulses) const
  {
    unsigned short per, wid;
    CVTimeUnits unit;

    try {
      OutputConf(cvOutput0);
      OutputConf(cvOutput1);
      OutputConf(cvOutput2);
      OutputConf(cvOutput3);
    } catch (Exception& e) {
      e.Dump();
    }

    // in us!
    if (width<0xFF*25.e-3) { // 6.375 us
      unit = cvUnit25ns;
      wid = static_cast<unsigned short>(width*1000/25);
      per = static_cast<unsigned short>(period*1000/25);
    }
    else if (width<0xFF*1.6) { // 408 us
      unit = cvUnit1600ns;
      wid = static_cast<unsigned short>(width*1000/1600);
      per = static_cast<unsigned short>(period*1000/1600);
    }
    else if (width<0xFF*410.) { // 104.55 ms
      unit = cvUnit410us;
      wid = static_cast<unsigned short>(width/410);
      per = static_cast<unsigned short>(period/410);
    }
    else if (width<0xFF*104.e3)  { // 26.52 s
      unit = cvUnit104ms;
      wid = static_cast<unsigned short>(width/104e3);
      per = static_cast<unsigned short>(period/104e3);
    }
    else throw Exception(__PRETTY_FUNCTION__, "Unsupported pulser width!", JustWarning);

    std::cout << width << " / " << period << " --> " << wid << " / " << per << std::endl;

    if (CAENVME_SetPulserConf(fHandle, cvPulserB, per, wid, unit, num_pulses, cvManualSW, cvManualSW)!=cvSuccess) {
      throw Exception(__PRETTY_FUNCTION__, "Failed to configure the pulser", JustWarning);
    }

    if (CAENVME_StartPulser(fHandle, cvPulserB)!=cvSuccess) {
      throw Exception(__PRETTY_FUNCTION__, "Failed to start the pulser", JustWarning);
    }
  }

  void
  BridgeVx718::StopPulser() const
  {
    if (CAENVME_StopPulser(fHandle, cvPulserB)!=cvSuccess) {
      throw Exception(__PRETTY_FUNCTION__, "Failed to stop the pulser", JustWarning);
    }
  }

  void
  BridgeVx718::WriteRegister(mod_reg addr, const uint16_t& data) const
  {
    uint32_t address = fBaseAddr+addr;
    uint16_t* fdata = new uint16_t; *fdata = data;
    if (CAENVME_WriteCycle(fHandle, address, fdata, cvA32_U_DATA, cvD16)!=cvSuccess) {
      std::ostringstream o; o << "Impossible to write register at 0x" << std::hex << addr;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  BridgeVx718::WriteRegister(mod_reg addr, const uint32_t& data) const
  {
    uint32_t address = fBaseAddr+addr;
    uint32_t* fdata = new uint32_t; *fdata = data;
    if (CAENVME_WriteCycle(fHandle, address, fdata, cvA32_U_DATA, cvD32)!=cvSuccess) {
      std::ostringstream o; o << "Impossible to write register at 0x" << std::hex << addr;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  BridgeVx718::ReadRegister(mod_reg addr, uint16_t* data) const
  {
    uint32_t address = fBaseAddr+addr;
    if (CAENVME_ReadCycle(fHandle, address, data, cvA32_U_DATA, cvD16)!=cvSuccess) {
      std::ostringstream o; o << "Impossible to read register at 0x" << std::hex << addr;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  BridgeVx718::ReadRegister(mod_reg addr, uint32_t* data) const
  {
    uint32_t address = fBaseAddr+addr;
    if (CAENVME_ReadCycle(fHandle, address, data, cvA32_U_DATA, cvD32)!=cvSuccess) {
      std::ostringstream o; o << "Impossible to read register at 0x" << std::hex << addr;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }
}
