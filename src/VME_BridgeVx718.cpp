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
    CVErrorCodes out = CAENVME_SetOutputConf(fHandle, output, cvDirect, cvActiveHigh, cvManualSW);
    if (out!=cvSuccess) {
      std::ostringstream o;
      o << "Failed to configure output register #" << static_cast<int>(output) << "\n\t"
        << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  // output := cvOutput[0,4]
  void
  BridgeVx718::OutputOn(unsigned short output) const
  {
    CVErrorCodes out = CAENVME_SetOutputRegister(fHandle, output);
    if (out!=cvSuccess) {
      std::ostringstream o;
      o << "Failed to enable output register #" << static_cast<int>(output) << "\n\t"
        << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  BridgeVx718::OutputOff(unsigned short output) const
  {
    CVErrorCodes out = CAENVME_ClearOutputRegister(fHandle, output);
    if (out!=cvSuccess) {
      std::ostringstream o;
      o << "Failed to disable output register #" << static_cast<int>(output) << "\n\t"
        << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  // input := cvInput[0,1]
  void
  BridgeVx718::InputConf(CVInputSelect input) const
  {
    CVErrorCodes out = CAENVME_SetInputConf(fHandle, input, cvDirect, cvActiveHigh);
    if (out!=cvSuccess) {
      std::ostringstream o;
      o << "Failed to configure input register #" << static_cast<int>(input) << "\n\t"
        << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  BridgeVx718::InputRead(CVInputSelect input) const
  {
    short unsigned int data;
    CVErrorCodes out = CAENVME_ReadRegister(fHandle, cvInputReg, &data);
    if (out!=cvSuccess) {
      std::ostringstream o;
      o << "Failed to read data input register #" << static_cast<int>(input) << "\n\t"
        << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
    // decoding with CVInputRegisterBits
    std::cout << "Input line 0 status: " << ((data&cvIn0Bit) >> 0) << std::endl;
    std::cout << "Input line 1 status: " << ((data&cvIn1Bit) >> 1) << std::endl;
  }

  void
  BridgeVx718::StartPulser(double period, double width, unsigned int num_pulses) const
  {
    unsigned char per, wid, np;
    CVTimeUnits unit;
    CVErrorCodes out;
    CVPulserSelect pulser = cvPulserA;
    CVIOSources start = cvManualSW, stop = cvManualSW;

    try {
      //OutputConf(cvOutput0);
      /*OutputConf(cvOutput1);
      OutputConf(cvOutput2);
      OutputConf(cvOutput3);*/
    } catch (Exception& e) {
      e.Dump();
    }

    // in us!
    if (width<0xFF*25.e-3) { // 6.375 us
      unit = cvUnit25ns;
      wid = static_cast<unsigned char>(width*1000/25);
      per = static_cast<unsigned char>(period*1000/25);
    }
    else if (width<0xFF*1.6) { // 408 us
      unit = cvUnit1600ns;
      wid = static_cast<unsigned char>(width*1000/1600);
      per = static_cast<unsigned char>(period*1000/1600);
    }
    else if (width<0xFF*410.) { // 104.55 ms
      unit = cvUnit410us;
      wid = static_cast<unsigned char>(width/410);
      per = static_cast<unsigned char>(period/410);
    }
    else if (width<0xFF*104.e3)  { // 26.52 s
      unit = cvUnit104ms;
      wid = static_cast<unsigned char>(width/104e3);
      per = static_cast<unsigned char>(period/104e3);
    }
    else throw Exception(__PRETTY_FUNCTION__, "Unsupported pulser width!", JustWarning);
    np = static_cast<unsigned char>(num_pulses&0xFF);

    std::cout << width << " / " << period << " --> " << static_cast<unsigned short>(wid&0xFF) << " / " << static_cast<unsigned short>(per&0xFF) << std::endl;

    out = CAENVME_SetPulserConf(fHandle, pulser, per, wid, unit, np, start, stop);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to configure the pulser" << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }
    
    out = CAENVME_GetPulserConf(fHandle, pulser, &per, &wid, &unit, &np, &start, &stop);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to retrieve the pulser configuration" << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }

    std::ostringstream os;
    os << "Starting a pulser with:" << "\n\t"
       << "  Pulse width:      " << width << " us" << "\n\t"
       << "  Period:           " << period << " us" << "\n\t"
       << "  Number of pulses: " << num_pulses << " (0 means infty)" << "\n\t"
       << "  Debug: " << static_cast<unsigned short>(per) << " / "
                      << static_cast<unsigned short>(wid) << " / "
                      << static_cast<unsigned short>(np) << " / ("
                      << static_cast<unsigned short>(unit) << " / "
                      << static_cast<unsigned short>(start) << " / "
                      << static_cast<unsigned short>(stop) << ")";
    PrintInfo(os.str());
    
    /*out = CAENVME_StartPulser(fHandle, pulser);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to start the pulser" << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
    }*/
  }

  void
  BridgeVx718::StopPulser() const
  {
    CVPulserSelect pulser = cvPulserA;
    if (CAENVME_StopPulser(fHandle, pulser)!=cvSuccess) {
      throw Exception(__PRETTY_FUNCTION__, "Failed to stop the pulser", JustWarning);
    }
  }

  void
  BridgeVx718::SinglePulse(unsigned short channel) const
  {
    unsigned short mask = 0x0;
    CVErrorCodes out;
    switch (channel) {
      case 0: mask |= cvOut0Bit; break;
      case 1: mask |= cvOut1Bit; break;
      case 2: mask |= cvOut2Bit; break;
      case 3: mask |= cvOut3Bit; break;
      case 4: mask |= cvOut4Bit; break;
      default: throw Exception(__PRETTY_FUNCTION__, "Trying to pulse on an undefined channel", JustWarning);
    }
    OutputConf(static_cast<CVOutputSelect>(channel));
    out = CAENVME_PulseOutputRegister(fHandle, mask);
    if (out!=cvSuccess) {
      std::ostringstream o;
      o << "Impossible to single-pulse output channel " << channel << "\n\t"
        << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  BridgeVx718::TestOutputs() const
  {
    /*bool on = false;
    OutputConf(cvOutput0); OutputConf(cvOutput1); OutputConf(cvOutput2); OutputConf(cvOutput3);
    while (true) {
      if (!on) {
        OutputOn(cvOut0Bit|cvOut1Bit|cvOut2Bit|cvOut3Bit);
        on = true;
        sleep(1);
      }
      else {
        OutputOff(cvOut0Bit|cvOut1Bit|cvOut2Bit|cvOut3Bit);
        on = false;
        sleep(1);
      }
    }*/
    while (true) {
      SinglePulse(0); sleep(1);
    }
  }

  void
  BridgeVx718::WriteRegister(CVRegisters addr, const uint16_t& data) const
  {
    uint32_t address = fBaseAddr+addr;
    uint16_t* fdata = new uint16_t; *fdata = data;
    CVErrorCodes out = CAENVME_WriteCycle(fHandle, address, fdata, cvA32_U_DATA, cvD16);
    if (out!=cvSuccess) {
      std::ostringstream o;
      o << "Impossible to write register at 0x" << std::hex << addr << "\n\t"
        << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  BridgeVx718::WriteRegister(CVRegisters addr, const uint32_t& data) const
  {
    uint32_t address = fBaseAddr+addr;
    uint32_t* fdata = new uint32_t; *fdata = data;
    CVErrorCodes out = CAENVME_WriteCycle(fHandle, address, fdata, cvA32_U_DATA, cvD32);
    if (out!=cvSuccess) {
      std::ostringstream o;
      o << "Impossible to write register at 0x" << std::hex << addr << "\n\t"
        << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  BridgeVx718::ReadRegister(CVRegisters addr, uint16_t* data) const
  {
    uint32_t address = fBaseAddr+addr;
    CVErrorCodes out = CAENVME_ReadCycle(fHandle, address, data, cvA32_U_DATA, cvD16);
    if (out!=cvSuccess) {
      std::ostringstream o;
      o << "Impossible to read register at 0x" << std::hex << addr << "\n\t"
        << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  BridgeVx718::ReadRegister(CVRegisters addr, uint32_t* data) const
  {
    uint32_t address = fBaseAddr+addr;
    CVErrorCodes out = CAENVME_ReadCycle(fHandle, address, data, cvA32_U_DATA, cvD32);
    if (out!=cvSuccess) {
      std::ostringstream o;
      o << "Impossible to read register at 0x" << std::hex << addr << "\n\t"
        << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }
}
