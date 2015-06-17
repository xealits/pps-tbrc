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
    
    //Map output lines [0,4] to corresponding register.
    fPortMapping[cvOutput0] = cvOut0Bit;
    fPortMapping[cvOutput1] = cvOut1Bit;
    fPortMapping[cvOutput2] = cvOut2Bit;
    fPortMapping[cvOutput3] = cvOut3Bit;
    fPortMapping[cvOutput4] = cvOut4Bit;

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
  BridgeVx718::OutputConf(CVOutputSelect output)
  {
    if (CAENVME_SetOutputConf(fHandle, output, cvDirect, cvActiveHigh, cvManualSW)!=cvSuccess) {
      std::ostringstream o;
      o << "Failed to configure output register #" << static_cast<int>(output);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  // output := cvOutput[0,4]
  void
  BridgeVx718::OutputOn(CVOutputSelect output)
  {
    if (CAENVME_SetOutputRegister(fHandle, fPortMapping[output])!=cvSuccess) {
      std::ostringstream o;
      o << "Failed to enable output register #" << static_cast<int>(output);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  BridgeVx718::OutputOff(CVOutputSelect output)
  {
    if (CAENVME_ClearOutputRegister(fHandle, fPortMapping[output])!=cvSuccess) {
      std::ostringstream o;
      o << "Failed to disable output register #" << static_cast<int>(output);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  // input := cvInput[0,1]
  void
  BridgeVx718::InputConf(CVInputSelect input)
  {
    if (CAENVME_SetInputConf(fHandle, input, cvDirect, cvActiveHigh)!=cvSuccess) {
      std::ostringstream o;
      o << "Failed to configure input register #" << static_cast<int>(input);
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  BridgeVx718::InputRead(CVInputSelect input)
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
}
