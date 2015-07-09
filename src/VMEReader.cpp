#include "VMEReader.h"

VMEReader::VMEReader(const char *device, VME::BridgeType type, bool on_socket) :
  Client(1987), fSG(0), fFPGA(0), fOnSocket(on_socket), fIsPulserStarted(false)
{
  try {
    if (fOnSocket) Client::Connect(DETECTOR);
    fBridge = new VME::BridgeVx718(device, type);
    //fBridge->TestOutputs();
    //StopPulser();
  } catch (Exception& e) {
    e.Dump();
    if (fOnSocket) Client::Send(e);
  }
}

VMEReader::~VMEReader()
{
  if (fOnSocket) Client::Disconnect();
  if (fSG) delete fSG;
  if (fFPGA) delete fFPGA;
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
    if (fOnSocket) Client::Send(e);
  }
  return 0;
}

void
VMEReader::AddTDC(uint32_t address)
{
  if (!fBridge) throw Exception(__PRETTY_FUNCTION__, "No bridge detected! Aborting...", Fatal);
  try {
    fTDCCollection.insert(std::pair<uint32_t,VME::TDCV1x90*>(
      address,
      new VME::TDCV1x90(fBridge->GetHandle(), address)
    ));
  } catch (Exception& e) {
    e.Dump();
    if (fOnSocket) Client::Send(e);
  }
}

void
VMEReader::AddIOModule(uint32_t address)
{
  if (!fBridge) throw Exception(__PRETTY_FUNCTION__, "No bridge detected! Aborting...", Fatal);
  try {
    fSG = new VME::IOModuleV262(fBridge->GetHandle(), address);
  } catch (Exception& e) {
    e.Dump();
    if (fOnSocket) Client::Send(e);
  }
}

void
VMEReader::AddFPGAUnit(uint32_t address)
{
  if (!fBridge) throw Exception(__PRETTY_FUNCTION__, "No bridge detected! Aborting...", Fatal);
  try {
    fFPGA = new VME::FPGAUnitV1495(fBridge->GetHandle(), address);
  } catch (Exception& e) {
    e.Dump();
    if (fOnSocket) Client::Send(e);
  }
  sleep(5); // wait for FW to be ready...
}

void
VMEReader::Abort()
{
  if (!fBridge) throw Exception(__PRETTY_FUNCTION__, "No bridge detected! Aborting...", Fatal);
  try {
    for (TDCCollection::iterator t=fTDCCollection.begin(); t!=fTDCCollection.end(); t++) {
      t->second->abort();
    }
  } catch (Exception& e) {
    e.Dump();
    if (fOnSocket) Client::Send(e);
  }
}
