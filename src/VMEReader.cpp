#include "VMEReader.h"

VMEReader::VMEReader(const char *device, VME::BridgeType type, bool on_socket) :
  Client(1987), fBridge(0), fSG(0), fFPGA(0), fCAENET(0), fHV(0),
  fOnSocket(on_socket), fIsPulserStarted(false),
  fOutputFile("")
{
  try {
    if (fOnSocket) Client::Connect(DETECTOR);
    fBridge = new VME::BridgeVx718(device, type);
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
  if (fBridge) delete fBridge;
  if (fHV) delete fHV;
  if (fCAENET) delete fCAENET;
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
VMEReader::AddCFD(uint32_t address)
{
  if (!fBridge) throw Exception(__PRETTY_FUNCTION__, "No bridge detected! Aborting...", Fatal);
  try {
    fCFDCollection.insert(std::pair<uint32_t,VME::CFDV812*>(
      address,
      new VME::CFDV812(fBridge->GetHandle(), address)
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
  sleep(4); // wait for FW to be ready...
}

void
VMEReader::AddHVModule(uint32_t vme_address, uint16_t nim_address)
{
  if (!fBridge) throw Exception(__PRETTY_FUNCTION__, "No bridge detected! Aborting...", Fatal);
  if (!fCAENET) fCAENET = new VME::CAENETControllerV288(fBridge->GetHandle(), vme_address);
  try {
    fHV = new NIM::HVModuleN470(nim_address, *fCAENET);
  } catch (Exception& e) {
    e.Dump();
    if (fOnSocket) Client::Send(e);
    std::ostringstream os;
    os << "Failed to add NIM HV module at address   0x" << std::hex << nim_address << "\n\t"
       << "through VME CAENET controller at address 0x" << std::hex << vme_address;
    throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
  }
}

void
VMEReader::Abort()
{
  if (!fBridge) throw Exception(__PRETTY_FUNCTION__, "No bridge detected! Aborting...", Fatal);
  try {
    for (VME::TDCCollection::iterator t=fTDCCollection.begin(); t!=fTDCCollection.end(); t++) {
      t->second->abort();
    }
  } catch (Exception& e) {
    e.Dump();
    if (fOnSocket) Client::Send(e);
  }
}

void
VMEReader::SetOutputFile(std::string filename)
{
  if (fOnSocket) Client::Send(SocketMessage(SET_NEW_FILENAME, filename.c_str()));
  fOutputFile = filename;
}
