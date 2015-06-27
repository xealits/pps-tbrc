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
  
  const unsigned int num_tdc = 2;

  fstream out_file[num_tdc];

  unsigned int num_events[num_tdc];
  VME::TDCEventCollection ec;
  VME::TDCV1x90* tdc[num_tdc];

  //VME::AcquisitionMode acq_mode = VME::CONT_STORAGE;
  VME::AcquisitionMode acq_mode = VME::TRIG_MATCH;
  VME::DetectionMode det_mode = VME::TRAILEAD;
  
  file_header_t fh;
  fh.magic = 0x30535050; // PPS0 in ASCII
  fh.run_id = 0;
  fh.spill_id = 0;
  fh.acq_mode = acq_mode;
  fh.det_mode = det_mode;
  
  std::time_t t_beg;
  try {
    bool with_socket = false;
    //vme = new VMEReader("/dev/usb/v1718_0", VME::CAEN_V1718, with_socket);
    vme = new VMEReader("/dev/a2818_0", VME::CAEN_V2718, with_socket);
    //vme->SendPulse();
    //vme->StartPulser(1000000., 200000.);
    
    fh.run_id = vme->GetRunNumber();
    
    vme->AddFPGAUnit(0xcc000000);
    //exit(0);

    // TDC configuration
    const uint32_t tdc_address[num_tdc] = { 0xaa000000, 0xbb000000 }; // V1290A (32 ch., CERN)
    
    for (unsigned int i=0; i<num_tdc; i++) {
      vme->AddTDC(tdc_address[i]);
      tdc[i] = vme->GetTDC(tdc_address[i]);
      tdc[i]->SetVerboseLevel(0);
      tdc[i]->SetAcquisitionMode(acq_mode);
      tdc[i]->SetDetectionMode(det_mode);
      tdc[i]->SetDLLClock(VME::TDCV1x90::DLL_PLL_HighRes);
      //tdc[i]->SetETTT();
    
      std::ostringstream filename;
      filename << "events_board" << i << ".dat";
      //filename << GenerateFileName(0);
      out_file[i].open(filename.str().c_str(), fstream::out | ios::binary );	
      if (!out_file[i].is_open()) {
        throw Exception(__PRETTY_FUNCTION__, "Error opening file", Fatal);
      }
      out_file[i].write((char*)&fh, sizeof(file_header_t));
      num_events[i] = 0;
   }
 
    std::string acqmode, detmode;
    switch (tdc[0]->GetAcquisitionMode()) {
      case VME::CONT_STORAGE: acqmode = "Continuous storage"; break;
      case VME::TRIG_MATCH: acqmode = "Trigger matching"; break;
      default:
        acqmode = "[Invalid mode]";
        throw Exception(__PRETTY_FUNCTION__, "Invalid acquisition mode!", Fatal);
    }
    switch (tdc[0]->GetDetectionMode()) {
      case VME::PAIR: detmode = "Pair measurement"; break;
      case VME::OLEADING: detmode = "Leading edge only"; break;
      case VME::OTRAILING: detmode = "Trailing edge only"; break;
      case VME::TRAILEAD: detmode = "Leading and trailing edges"; break;
    }

    t_beg = std::time(0);

    cerr << endl << "*** Ready for acquisition! ***" << endl
         << "Acquisition mode: " << acqmode << endl 
         << "Detection mode: " << detmode << endl 
         << "Local time: " << asctime(std::localtime(&t_beg));

    vme->SendPulse(0); // send a CLR signal from bridge to TDC

    while (true) {
      for (unsigned int i=0; i<num_tdc; i++) {
        ec = tdc[i]->FetchEvents();
        if (ec.size()==0) continue; // no events were fetched
        for (VME::TDCEventCollection::const_iterator e=ec.begin(); e!=ec.end(); e++) {
          out_file[i].write((char*)&(*e), sizeof(VME::TDCEvent));
        }
        num_events[i] += ec.size();
      }
    }
    //while(true) {;}
  } catch (Exception& e) {
    if (e.ErrorNumber()==TDC_ACQ_STOP) {
      for (unsigned int i=0; i<num_tdc; i++) {
        if (out_file[i].is_open()) out_file[i].close();
      }

      std::time_t t_end = std::time(0);
      double nsec_tot = difftime(t_end, t_beg), nsec = fmod(nsec_tot,60), nmin = (nsec_tot-nsec)/60.;
      cerr << endl << "*** Acquisition stopped! ***" << endl
           << "Local time: " << asctime(std::localtime(&t_end))
           << "Total acquisition time: " << difftime(t_end, t_beg) << " seconds"
           << " (" << nmin << " min " << nsec << " sec)"
           << endl;
    
      cerr << endl << "Acquired ";
      for (unsigned int i=0; i<num_tdc; i++) {
        if (i>0) cerr << " / ";
        cerr << num_events[i];
      }
      cerr << " words in this run" << endl;
  
      delete vme;
      return 0;
    }
    e.Dump();
    return -1;
  }
    
  return 0;
}
