#include "VMEReader.h"
#include "FileConstants.h"

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <ctime>
#include <signal.h>

using namespace std;

VMEReader* vme;
fstream out_file;
int gEnd = 0;

void CtrlC(int aSig) {
  if (gEnd==0) { cerr << endl << "[C-c] Trying a clean exit!" << endl;
    vme->Abort();
  }
  else if (gEnd>=5) { cerr << endl << "[C-c > 5 times] ... Forcing exit!" << endl;
    exit(0);
  }
  gEnd++;
}

int main(int argc, char *argv[]) {
  signal(SIGINT, CtrlC);
  
  unsigned int num_events;
  VME::TDCEventCollection ec;
  VME::TDCV1x90* tdc;
  string filename;

  VME::AcquisitionMode acq_mode = VME::CONT_STORAGE;
  VME::DetectionMode det_mode = VME::TRAILEAD;
  
  file_header_t fh;
  fh.magic = 0x30535050; // PPS0 in ASCII
  fh.run_id = 0;
  fh.spill_id = 0;
  fh.acq_mode = acq_mode;
  fh.det_mode = det_mode;
  
  std::time_t t_beg;
  num_events = 0;
  try {
    bool with_socket = false;
    //vme = new VMEReader("/dev/usb/v1718_0", VME::CAEN_V1718, with_socket);
    vme = new VMEReader("/dev/a2818_0", VME::CAEN_V2718, with_socket);
    
    fh.run_id = vme->GetRunNumber();
    
    // TDC configuration
    //const uint32_t tdc_address = 0x000d0000; // V1290N (16 ch., Louvain-la-Neuve)
    const uint32_t tdc_address = 0x00aa0000; // V1290A (32 ch., CERN)
    //const uint32_t tdc_address = 0x00bb0000; // V1290A (32 ch., CERN)
    
    //vme->SendPulse();
    //vme->StartPulser(1000000., 200000.);

    vme->AddTDC(tdc_address);
    tdc = vme->GetTDC(tdc_address);
    //tdc->SetVerboseLevel(0);
    tdc->GetControl().Dump();
    tdc->SetAcquisitionMode(acq_mode);
    tdc->SetDetectionMode(det_mode);
    tdc->SetDLLClock(VME::TDCV1x90::DLL_PLL_HighRes);
    tdc->SetETTT();
    //tdc->SetTestMode();
    //tdc->SetWindowWidth(2040);
    //tdc->SetWindowOffset(-2045);
    
    filename = GenerateFileName(0);
    out_file.open(filename.c_str(), fstream::out | ios::binary );	
    if (!out_file.is_open()) {
      ostringstream o; o << "Error opening file " << filename;
      throw Exception(__PRETTY_FUNCTION__, o.str(), Fatal);
    }
    
    t_beg = std::time(0);
    std::string acqmode, detmode;
    switch (tdc->GetAcquisitionMode()) {
      case VME::CONT_STORAGE: acqmode = "Continuous storage"; break;
      case VME::TRIG_MATCH: acqmode = "Trigger matching"; break;
      default: acqmode = "[Invalid mode]"; throw Exception(__PRETTY_FUNCTION__, "Invalid acquisition mode!", Fatal);
    }
    switch (tdc->GetDetectionMode()) {
      case VME::PAIR: detmode = "Pair measurement"; break;
      case VME::OLEADING: detmode = "Leading edge only"; break;
      case VME::OTRAILING: detmode = "Trailing edge only"; break;
      case VME::TRAILEAD: detmode = "Leading and trailing edges"; break;
    }
    cerr << endl << "*** Ready for acquisition! ***" << endl
         << "Acquisition mode: " << acqmode << endl 
         << "Detection mode: " << detmode << endl 
         << "Local time: " << asctime(std::localtime(&t_beg));
    
    out_file.write((char*)&fh, sizeof(file_header_t));

    vme->SendPulse(0); // send a CLR signal from bridge to TDC

    while (true) {
      ec = tdc->FetchEvents();
      if (ec.size()==0) { // no events were fetched
        tdc->GetStatus().Dump();
        sleep(2);
        continue;
      }
      for (VME::TDCEventCollection::const_iterator e=ec.begin(); e!=ec.end(); e++) {
        //e->Dump();
        out_file.write((char*)&(*e), sizeof(VME::TDCEvent));
      }
      num_events += ec.size();
    }
    //while(true) {;}
  } catch (Exception& e) {
    if (e.ErrorNumber()==TDC_ACQ_STOP) {
      if (out_file.is_open()) out_file.close();
      std::time_t t_end = std::time(0);
      double nsec_tot = difftime(t_end, t_beg), nsec = fmod(nsec_tot,60), nmin = (nsec_tot-nsec)/60.;
      cerr << endl << "*** Acquisition stopped! ***" << endl
           << "Local time: " << asctime(std::localtime(&t_end))
           << "Total acquisition time: " << difftime(t_end, t_beg) << " seconds"
           << " (" << nmin << " min " << nsec << " sec)"
           << endl;
      out_file.close();
    
      cerr << endl << "Acquired " << num_events << " words in this run" << endl;
  
      delete vme;
      return 0;
    }
    e.Dump();
    return -1;
  }
    
  return 0;
}
