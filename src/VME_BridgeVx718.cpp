#include "VME_BridgeVx718.h"

namespace VME
{
  BridgeVx718::BridgeVx718(const char* device, BridgeType type) :
    GenericBoard<CVRegisters,cvA32_U_DATA>(0, 0x0)
  {
    int dev = atoi(device);
    CVBoardTypes tp;
    CVErrorCodes ret; 
    std::ostringstream o;
 
    /*char rel[20];
    CAENVME_SWRelease(rel);
    o.str("");
    o << "Initializing the VME bridge\n\t"
      << "CAEN library release: " << rel;
    PrintInfo(o.str());*/

    switch (type) {
      case CAEN_V1718: tp = cvV1718; break;
      case CAEN_V2718:
        tp = cvV2718;
        try { CheckPCIInterface(device); } catch (Exception& e) {
          e.Dump();
          throw Exception(__PRETTY_FUNCTION__, "Failed to initialize the PCI/VME interface card!", Fatal);
        }
        break;
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
      throw Exception(__PRETTY_FUNCTION__, o.str(), Fatal, CAEN_ERROR(ret));
    }

    char board_rel[100];
    ret = CAENVME_BoardFWRelease(fHandle, board_rel);
    if (ret!=cvSuccess) {
      o.str("");
      o << "Failed to retrieve the board FW release!\n\t"
        << "CAEN error: " << CAENVME_DecodeError(ret);
      throw Exception(__PRETTY_FUNCTION__, o.str(), Fatal, CAEN_ERROR(ret));
    }
    o.str("");
    o << "Bridge firmware version: " << board_rel;
    PrintInfo(o.str());
    CheckConfiguration();

    //SetIRQ(IRQ1|IRQ2|IRQ3|IRQ4|IRQ5|IRQ6|IRQ7, false);
  }

  BridgeVx718::~BridgeVx718()
  {
    CAENVME_End(fHandle);
  }

  void
  BridgeVx718::CheckPCIInterface(const char* device) const
  {
    try {
      PCIInterfaceA2818 pci(device);
      std::ostringstream os;
      os << "PCI/VME interface card information:" << "\n\t"
         << "  FW version: " << pci.GetFWRevision();
      PrintInfo(os.str()); 
    } catch (Exception& e) {
      throw e;
    }
  }

  void
  BridgeVx718::CheckConfiguration() const
  {
    CVErrorCodes ret;
    CVDisplay config;
    std::ostringstream o;
    ret = CAENVME_ReadDisplay(fHandle, &config);
    if (ret!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to retrieve configuration displayed on\n\t"
         << "module's front panel\n\t"
         << "CAEN error: " << CAENVME_DecodeError(ret);
      throw Exception(__PRETTY_FUNCTION__, os.str(), Fatal, CAEN_ERROR(ret));
    }
  }

  void
  BridgeVx718::Reset() const
  {
    CVErrorCodes out = CAENVME_SystemReset(fHandle);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to request status register" << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(out));
    }
    PrintInfo("Resetting the bridge module!");
  }

  BridgeVx718Status
  BridgeVx718::GetStatus() const
  {
    uint32_t data;
    CVErrorCodes out = CAENVME_ReadRegister(fHandle, cvStatusReg, &data);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to request status register" << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(out));
    }
    return BridgeVx718Status(data);
  }

  void
  BridgeVx718::SetIRQ(unsigned int irq, bool enable)
  {
    CVErrorCodes out;
    unsigned long word = static_cast<unsigned long>(irq);
    if (enable) out = CAENVME_IRQEnable (fHandle, word);
    else        out = CAENVME_IRQDisable(fHandle, word);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to set IRQ enable status to " << enable << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(out));
    }
    fHasIRQ = enable;
  }

  void
  BridgeVx718::WaitIRQ(unsigned int irq, unsigned long timeout) const
  {
    uint8_t word = static_cast<uint8_t>(irq);
    CVErrorCodes out = CAENVME_IRQWait(fHandle, word, timeout);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to request IRQ" << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(out));
    }
  }

  unsigned int
  BridgeVx718::GetIRQStatus() const
  {
    uint8_t mask;
    CVErrorCodes out = CAENVME_IRQCheck(fHandle, &mask);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to retrieve IRQ status" << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(out));
    }
    return static_cast<unsigned int>(mask);
  }

  // output := cvOutput[0,4] 
  void
  BridgeVx718::OutputConf(CVOutputSelect output) const
  {
    CVErrorCodes out = CAENVME_SetOutputConf(fHandle, output, cvDirect, cvActiveHigh, cvManualSW);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to configure output register #" << static_cast<int>(output) << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(out));
    }
  }

  // output := cvOutput[0,4]
  void
  BridgeVx718::OutputOn(unsigned short output) const
  {
    CVErrorCodes out = CAENVME_SetOutputRegister(fHandle, output);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to enable output register #" << static_cast<int>(output) << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(out));
    }
  }

  void
  BridgeVx718::OutputOff(unsigned short output) const
  {
    CVErrorCodes out = CAENVME_ClearOutputRegister(fHandle, output);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to disable output register #" << static_cast<int>(output) << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(out));
    }
  }

  // input := cvInput[0,1]
  void
  BridgeVx718::InputConf(CVInputSelect input) const
  {
    CVErrorCodes out = CAENVME_SetInputConf(fHandle, input, cvDirect, cvActiveHigh);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to configure input register #" << static_cast<int>(input) << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(out));
    }
  }

  void
  BridgeVx718::InputRead(CVInputSelect input) const
  {
    unsigned int data;
    CVErrorCodes out = CAENVME_ReadRegister(fHandle, cvInputReg, &data);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to read data input register #" << static_cast<int>(input) << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(out));
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
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(out));
    }
    
    out = CAENVME_GetPulserConf(fHandle, pulser, &per, &wid, &unit, &np, &start, &stop);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to retrieve the pulser configuration" << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(out));
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
    CVErrorCodes out =CAENVME_StopPulser(fHandle, pulser);
    if (out!=cvSuccess) {
      std::ostringstream os;
      os << "Failed to stop the pulser" << "\n\t"
         << "CAEN error: " << CAENVME_DecodeError(out);
      throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(out));
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
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning, CAEN_ERROR(out));
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
}
