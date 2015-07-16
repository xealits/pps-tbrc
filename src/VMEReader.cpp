#include "VMEReader.h"

VMEReader::VMEReader(const char *device, VME::BridgeType type, bool on_socket) :
  Client(1987), fSG(0), fFPGA(0), fOnSocket(on_socket), fIsPulserStarted(false),
  fOutputFile("")
{
  try {
    if (fOnSocket) Client::Connect(DETECTOR);
    //fBridge = new VME::BridgeVx718(device, type);
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

void
VMEReader::ReadXML(const char* filename)
{
  tinyxml2::XMLDocument doc;
  doc.LoadFile(filename);
  if (doc.Error()) {
    std::ostringstream os;
    os << "Error while trying to parse the configuration file" << "\n\t"
       << "Code: " << doc.ErrorID() << "\n\t"
       << "Dump of error:" << "\n"
       << doc.GetErrorStr1();
    throw Exception(__PRETTY_FUNCTION__, os.str(), Fatal);
  }
  if (tinyxml2::XMLElement* fpga=doc.FirstChildElement("fpga")) {
    if (const char* address=fpga->Attribute("address")) {
      unsigned int addr = static_cast<unsigned int>(strtol(address, NULL, 0));
      if (!addr) throw Exception(__PRETTY_FUNCTION__, "Failed to parse FPGA's base address", Fatal);
        std::cout << "--> 0x" << std::hex << addr << std::endl;
      try {
        AddFPGAUnit(addr);
        VME::FPGAUnitV1495Control control = fFPGA->GetControl();
        if (tinyxml2::XMLElement* clock=fpga->FirstChildElement("clock")) {
          if (tinyxml2::XMLElement* source=clock->FirstChildElement("source")) {
            if (!strcmp(source->GetText(),"internal")) control.SetClockSource(VME::FPGAUnitV1495Control::InternalClock);
            if (!strcmp(source->GetText(),"external")) control.SetClockSource(VME::FPGAUnitV1495Control::ExternalClock);
          }
          if (tinyxml2::XMLElement* period=clock->FirstChildElement("period")) {
            fFPGA->SetInternalClockPeriod(atoi(period->GetText()));
          }
        }
        if (tinyxml2::XMLElement* trig=fpga->FirstChildElement("trigger")) {
          if (tinyxml2::XMLElement* source=trig->FirstChildElement("source")) {
            if (!strcmp(source->GetText(),"internal")) control.SetTriggerSource(VME::FPGAUnitV1495Control::InternalTrigger);
            if (!strcmp(source->GetText(),"external")) control.SetTriggerSource(VME::FPGAUnitV1495Control::ExternalTrigger);
          }
          if (tinyxml2::XMLElement* period=trig->FirstChildElement("period")) {
            fFPGA->SetInternalTriggerPeriod(atoi(period->GetText()));
          }
        }
        if (tinyxml2::XMLElement* sig=fpga->FirstChildElement("signal")) {
          if (tinyxml2::XMLElement* source=sig->FirstChildElement("source")) {
            if (!strcmp(source->GetText(),"internal")) for (unsigned int i=0; i<4; i++) control.SetSignalSource(i, VME::FPGAUnitV1495Control::InternalSignal);
            if (!strcmp(source->GetText(),"external")) for (unsigned int i=0; i<4; i++) control.SetSignalSource(i, VME::FPGAUnitV1495Control::ExternalSignal);
          }
          if (tinyxml2::XMLElement* poi=sig->FirstChildElement("poi")) {
            fFPGA->SetOutputPulserPOI(atoi(poi->GetText()));
          }
        }
      } catch (Exception& e) { throw e; }
    }
    else throw Exception(__PRETTY_FUNCTION__, "Failed to extract FPGA's base address", Fatal);
  }
  if (tinyxml2::XMLElement* tdc=doc.FirstChildElement("tdc")) {
    if (const char* address=tdc->Attribute("address")) {
      unsigned int addr = static_cast<unsigned int>(strtol(address, NULL, 0));
      if (!addr) throw Exception(__PRETTY_FUNCTION__, "Failed to parse TDC's base address", Fatal);
        std::cout << "--> 0x" << std::hex << addr << std::endl;
    }
    PrintInfo("tdc");
  }
  //doc.Print();
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
  sleep(4); // wait for FW to be ready...
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
