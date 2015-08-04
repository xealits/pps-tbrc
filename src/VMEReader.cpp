#include "VMEReader.h"

VMEReader::VMEReader(const char *device, VME::BridgeType type, bool on_socket) :
  Client(1987), fBridge(0), fSG(0), fFPGA(0), fCAENET(0), fHV(0),
  fOnSocket(on_socket), fIsPulserStarted(false)
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
  if (fOnSocket) {
    Client::Send(Exception(__PRETTY_FUNCTION__, "Stopping the acquisition process", Info, 30001));
    Client::Disconnect();
  }
  if (fSG) delete fSG;
  if (fFPGA) delete fFPGA;
  if (fIsPulserStarted) fBridge->StopPulser();
  if (fBridge) delete fBridge;
  if (fHV) delete fHV;
  if (fCAENET) delete fCAENET;
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
  int triggering_mode = 0;
  if (tinyxml2::XMLElement* fpga=doc.FirstChildElement("triggering")) {
    if (const char* mode=fpga->Attribute("mode")) {
      std::ostringstream os; os << "Triggering mode: ";
      if (!strcmp(mode,"continuous_storage")) { triggering_mode = 0; os << "Continuous storage (manual)"; }
      if (!strcmp(mode,"trigger_start"))      { triggering_mode = 1; os << "Continuous storage (trigger on start)"; }
      if (!strcmp(mode,"trigger_matching"))   { triggering_mode = 2; os << "Trigger matching"; }
      if (fOnSocket) Client::Send(Exception(__PRETTY_FUNCTION__, os.str(), Info));
    }
  }
  if (tinyxml2::XMLElement* fpga=doc.FirstChildElement("fpga")) {
    if (const char* address=fpga->Attribute("address")) {
      unsigned long addr = static_cast<unsigned long>(strtol(address, NULL, 0));
      if (!addr) throw Exception(__PRETTY_FUNCTION__, "Failed to parse FPGA's base address", Fatal);
      try {
        try { AddFPGAUnit(addr); } catch (Exception& e) { if (fOnSocket) Client::Send(e); }
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
            if (!strcmp(source->GetText(),"internal")) for (unsigned int i=0; i<2; i++) control.SetSignalSource(i, VME::FPGAUnitV1495Control::InternalSignal);
            if (!strcmp(source->GetText(),"external")) for (unsigned int i=0; i<2; i++) control.SetSignalSource(i, VME::FPGAUnitV1495Control::ExternalSignal);
          }
          if (tinyxml2::XMLElement* poi=sig->FirstChildElement("poi")) {
            fFPGA->SetOutputPulserPOI(atoi(poi->GetText()));
          }
        }
        switch (triggering_mode) {
          case 0:
            control.SetTriggeringMode(VME::FPGAUnitV1495Control::ContinuousStorage); break;
            control.SetTriggerSource(VME::FPGAUnitV1495Control::ExternalTrigger); break;
          case 1:
            control.SetTriggeringMode(VME::FPGAUnitV1495Control::ContinuousStorage); break;
          case 2:
            control.SetTriggeringMode(VME::FPGAUnitV1495Control::TriggerMatching); break;
        }
        fFPGA->SetControl(control);
      } catch (Exception& e) { e.Dump(); if (fOnSocket) Client::Send(e); throw e; }
    }
    else throw Exception(__PRETTY_FUNCTION__, "Failed to extract FPGA's base address", Fatal);
  }
  unsigned int tdc_id = 0;
  for (tinyxml2::XMLElement* atdc=doc.FirstChildElement("tdc"); atdc!=NULL; atdc=atdc->NextSiblingElement("tdc"), tdc_id++) {
    if (const char* address=atdc->Attribute("address")) {
      unsigned long addr = static_cast<unsigned long>(strtol(address, NULL, 0));
      if (!addr) throw Exception(__PRETTY_FUNCTION__, "Failed to parse TDC's base address", Fatal);
      try {
        try { AddTDC(addr); } catch (Exception& e) { if (fOnSocket) Client::Send(e); }
        VME::TDCV1x90* tdc = GetTDC(addr);
        std::string detector_name = "unknown";
        switch (triggering_mode) {
          case 0:
          case 1:
            tdc->SetAcquisitionMode(VME::CONT_STORAGE); break;
          case 2:
            tdc->SetAcquisitionMode(VME::TRIG_MATCH); break;
        }
        if (tinyxml2::XMLElement* verb=atdc->FirstChildElement("verbosity")) {
          tdc->SetVerboseLevel(atoi(verb->GetText()));
        }
        if (tinyxml2::XMLElement* det=atdc->FirstChildElement("detector")) {
          detector_name = det->GetText();
        }
	if (tinyxml2::XMLElement* det=atdc->FirstChildElement("det_mode")) {
	  if (!strcmp(det->GetText(),"trailead")) tdc->SetDetectionMode(VME::TRAILEAD);
	  if (!strcmp(det->GetText(),"leading")) tdc->SetDetectionMode(VME::OLEADING);
	  if (!strcmp(det->GetText(),"trailing")) tdc->SetDetectionMode(VME::OTRAILING);
	  if (!strcmp(det->GetText(),"pair")) tdc->SetDetectionMode(VME::PAIR);
        }
	if (tinyxml2::XMLElement* dll=atdc->FirstChildElement("dll")) {
	  if (!strcmp(dll->GetText(),"Direct_Low_Resolution")) tdc->SetDLLClock(VME::TDCV1x90::DLL_Direct_LowRes);
	  if (!strcmp(dll->GetText(),"PLL_Low_Resolution")) tdc->SetDLLClock(VME::TDCV1x90::DLL_PLL_LowRes);
	  if (!strcmp(dll->GetText(),"PLL_Medium_Resolution")) tdc->SetDLLClock(VME::TDCV1x90::DLL_PLL_MedRes);
	  if (!strcmp(dll->GetText(),"PLL_High_Resolution")) tdc->SetDLLClock(VME::TDCV1x90::DLL_PLL_HighRes);
        }
	if (atdc->FirstChildElement("ettt")) { tdc->SetETTT(); }
	if (tinyxml2::XMLElement* wind=atdc->FirstChildElement("trigger_window")) {
          if (tinyxml2::XMLElement* width=wind->FirstChildElement("width")) { tdc->SetWindowWidth(atoi(width->GetText())); }
          if (tinyxml2::XMLElement* offset=wind->FirstChildElement("offset")) { tdc->SetWindowOffset(atoi(offset->GetText())); }
        }
        OnlineDBHandler("run_infos.db").SetTDCConditions(tdc_id, addr, tdc->GetAcquisitionMode(), tdc->GetDetectionMode(), detector_name);
      } catch (Exception& e) { throw e; }
    }
  }
  for (tinyxml2::XMLElement* acfd=doc.FirstChildElement("cfd"); acfd!=NULL; acfd=acfd->NextSiblingElement("cfd")) {
    if (const char* address=acfd->Attribute("address")) {
      unsigned long addr = static_cast<unsigned long>(strtol(address, NULL, 0));
      if (!addr) throw Exception(__PRETTY_FUNCTION__, "Failed to parse CFD's base address", Fatal);
      try {
        try { AddCFD(addr); } catch (Exception& e) { if (fOnSocket) Client::Send(e); }
        VME::CFDV812* cfd = GetCFD(addr);
        if (tinyxml2::XMLElement* poi=acfd->FirstChildElement("poi")) { cfd->SetPOI(atoi(poi->GetText())); }
        if (tinyxml2::XMLElement* ow=acfd->FirstChildElement("output_width")) {
          if (tinyxml2::XMLElement* g0=ow->FirstChildElement("group0")) { cfd->SetOutputWidth(0, atoi(g0->GetText())); }
          if (tinyxml2::XMLElement* g1=ow->FirstChildElement("group1")) { cfd->SetOutputWidth(1, atoi(g1->GetText())); }
        }
        if (tinyxml2::XMLElement* dt=acfd->FirstChildElement("dead_time")) {
          if (tinyxml2::XMLElement* g0=dt->FirstChildElement("group0")) { cfd->SetDeadTime(0, atoi(g0->GetText())); }
          if (tinyxml2::XMLElement* g1=dt->FirstChildElement("group1")) { cfd->SetDeadTime(1, atoi(g1->GetText())); }
        }
        if (tinyxml2::XMLElement* thr=acfd->FirstChildElement("threshold")) {
          for (tinyxml2::XMLElement* ch=thr->FirstChildElement("channel"); ch!=NULL; ch=ch->NextSiblingElement("channel")) {
            cfd->SetThreshold(atoi(ch->Attribute("id")), atoi(ch->GetText()));
          }
        }
      } catch (Exception& e) { throw e; }
    }
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
  std::ostringstream os;
  os << "TDC with base address 0x" << std::hex << address << " successfully built";
  throw Exception(__PRETTY_FUNCTION__, os.str(), Info, TDC_ACQ_START);
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
  std::ostringstream os; os << "CFD with base address 0x" << std::hex << address << " successfully built";
  throw Exception(__PRETTY_FUNCTION__, os.str(), Info, TDC_ACQ_START);
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
  std::ostringstream os; os << "I/O module with base address 0x" << std::hex << address << " successfully built";
  throw Exception(__PRETTY_FUNCTION__, os.str(), Info, TDC_ACQ_START);
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
  std::ostringstream os; os << "FPGA module with base address 0x" << std::hex << address << " successfully built";
  throw Exception(__PRETTY_FUNCTION__, os.str(), Info, TDC_ACQ_START);
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
  std::ostringstream os;
  os << "NIM HV module with address 0x" << std::hex << nim_address << "\n\t"
     << " (through VME CAENET controller at base address 0x" << std::hex << vme_address << ") successfully built";
  throw Exception(__PRETTY_FUNCTION__, os.str(), Info, TDC_ACQ_START);
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
VMEReader::SetOutputFile(uint32_t tdc_address, std::string filename)
{
  OutputFiles::iterator it = fOutputFiles.find(tdc_address);
  if (it!=fOutputFiles.end()) { it->second = filename; }
  else fOutputFiles.insert(std::pair<uint32_t, std::string>(tdc_address, filename));
}

void
VMEReader::SendOutputFile(uint32_t tdc_address, unsigned int spill_id) const
{
  if (!fOnSocket) return;
  OutputFiles::const_iterator it = fOutputFiles.find(tdc_address);
  if (it!=fOutputFiles.end()) {
    std::ostringstream os;
    os.str(""); os << "New spill detected: " << spill_id;
    Client::Send(Exception(__PRETTY_FUNCTION__, os.str(), JustWarning));
    os.str(""); os << tdc_address << ":" << it->second;
    Client::Send(SocketMessage(SET_NEW_FILENAME, os.str()));
  }
}
