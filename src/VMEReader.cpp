#include "VMEReader.h"

VMEReader::VMEReader(const char *device, unsigned int type) :
  Client(1987)
{
  Client::Connect();
  fBridge = new VME::BridgeVx718(device, type);
  fTDC = new VME::TDCV1x90(fBridge->GetHandle(), 0x000d0000, VME::TRIG_MATCH, VME::TRAILEAD);
  fTDC->GetFirmwareRev();
}

VMEReader::~VMEReader()
{
  delete fTDC;
  delete fBridge;
}

unsigned int
VMEReader::GetRunNumber()
{
  SocketMessage msg;
  try {
    msg = Client::SendAndReceive(SocketMessage(GET_RUN_NUMBER), RUN_NUMBER);
    return static_cast<unsigned int>(msg.GetIntValue());
  } catch (Exception& e) {
    e.Dump();
  }
  return 0;
}
