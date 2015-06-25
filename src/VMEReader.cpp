#include "VMEReader.h"

VMEReader::VMEReader(const char *device, VME::BridgeType type, bool on_socket) :
  Client(1987), fSG(0), fOnSocket(on_socket), fIsPulserStarted(false)
{
  if (fOnSocket) {
    Client::Connect();
  }
  fBridge = new VME::BridgeVx718(device, type);
  //fBridge->TestOutputs();

  try {
    StopPulser();
  } catch (Exception& e) { e.Dump(); }
}

VMEReader::~VMEReader()
{
  if (fSG) delete fSG;
  if (fIsPulserStarted) fBridge->StopPulser();
  delete fBridge;
}

unsigned int
VMEReader::GetRunNumber()
{
  if (!fOnSocket) return 0;
  
  SocketMessage msg;
  try {
    msg = Client::SendAndReceive(SocketMessage(GET_RUN_NUMBER), RUN_NUMBER);
    return static_cast<unsigned int>(msg.GetIntValue());
  } catch (Exception& e) {
    e.Dump();
  }
  return 0;
}

void
VMEReader::AddTDC(uint32_t address)
{
  VME::TDCV1x90* tdc = new VME::TDCV1x90(fBridge->GetHandle(), address, VME::TRIG_MATCH, VME::TRAILEAD);
  tdc->GetFirmwareRevision();
  fTDCCollection.insert(std::pair<uint32_t,VME::TDCV1x90*>(address, tdc));
}

void
VMEReader::AddIOModule(uint32_t address)
{
  fSG = new VME::IOModuleV262(fBridge->GetHandle(), address);
}

void
VMEReader::Abort()
{
  for (TDCCollection::iterator t=fTDCCollection.begin(); t!=fTDCCollection.end(); t++) {
    t->second->abort();
  }
}
