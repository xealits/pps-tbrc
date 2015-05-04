#include "VMEBridgeV1718.h"

VMEBridgeV1718::VMEBridgeV1718(const char *device)
{
  int dev = atoi(device); 
  if(CAENVME_Init(cvV1718, 0, dev, &bhandle) != cvSuccess) {
    throw Exception(__PRETTY_FUNCTION__, "Error opening the VME bridge", Fatal);
  }
#ifdef DEBUG
  std::cout << "[VME] <Bridge::Bridge> Debug: BHandle " << (int)bhandle << std::endl;
#endif
  
  //Map output lines [0,4] to corresponding register.
  map_port[cvOutput0] = cvOut0Bit;
  map_port[cvOutput1] = cvOut1Bit;
  map_port[cvOutput2] = cvOut2Bit;
  map_port[cvOutput3] = cvOut3Bit;
  map_port[cvOutput4] = cvOut4Bit;
}

int32_t
VMEBridgeV1718::GetBHandle()
{
#ifdef DEBUG
  std::cout << "[VME] <Bridge::getBHandle> Debug: BHandle " << (int)bhandle << std::endl;
#endif
  return bhandle;
}

VMEBridgeV1718::~VMEBridgeV1718() {
  CAENVME_End(bhandle);
}

// output := cvOutput[0,4] 
void
VMEBridgeV1718::OutputConf(CVOutputSelect output)
{
  if(CAENVME_SetOutputConf(bhandle,output,cvDirect,cvActiveHigh,cvManualSW) != cvSuccess) {
    std::ostringstream o;
    o << "Failed to configure output register #" << static_cast<int>(output);
    throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
  }
}

// output := cvOutput[0,4]
void
VMEBridgeV1718::OutputOn(CVOutputSelect output)
{
  if(CAENVME_SetOutputRegister(bhandle,map_port[output]) != cvSuccess) {
    std::ostringstream o;
    o << "Failed to enable output register #" << static_cast<int>(output);
    throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
  }
}

void
VMEBridgeV1718::OutputOff(CVOutputSelect output)
{
  if(CAENVME_ClearOutputRegister(bhandle,map_port[output]) != cvSuccess) {
    std::ostringstream o;
    o << "Failed to disable output register #" << static_cast<int>(output);
    throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
  }
}

// input := cvInput[0,1]
void
VMEBridgeV1718::InputConf(CVInputSelect input)
{
  if(CAENVME_SetInputConf(bhandle,input,cvDirect,cvActiveHigh) != cvSuccess) {
    std::ostringstream o;
    o << "Failed to configure input register #" << static_cast<int>(input);
    throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
  }
}

void
VMEBridgeV1718::InputRead(CVInputSelect input)
{
  unsigned int data;
  if(CAENVME_ReadRegister(bhandle,cvInputReg,&data) != cvSuccess) {
    std::ostringstream o;
    o << "Failed to read data input register #" << static_cast<int>(input);
    throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
  }
  // decoding with CVInputRegisterBits
  std::cout << "Input line 0 status: " << ((data&cvIn0Bit) >> 0) << std::endl;
  std::cout << "Input line 1 status: " << ((data&cvIn1Bit) >> 1) << std::endl;
}
