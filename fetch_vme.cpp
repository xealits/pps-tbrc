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
  
  fstream out_file_board1, out_file_board2;

  unsigned int num_events_board1, num_events_board2;
  VME::TDCEventCollection ec;
  VME::TDCV1x90 *tdc1, *tdc2;

  VME::AcquisitionMode acq_mode = VME::CONT_STORAGE;
  VME::DetectionMode det_mode = VME::TRAILEAD;
  
  file_header_t fh;
  fh.magic = 0x30535050; // PPS0 in ASCII
  fh.run_id = 0;
  fh.spill_id = 0;
  fh.acq_mode = acq_mode;
  fh.det_mode = det_mode;
  
  std::time_t t_beg;
  num_events_board1 = num_events_board2 = 0;
  try {
    bool with_socket = false;
    //vme = new VMEReader("/dev/usb/v1718_0", VME::CAEN_V1718, with_socket);
    vme = new VMEReader("/dev/a2818_0", VME::CAEN_V2718, with_socket);
    
    fh.run_id = vme->GetRunNumber();
    
    // TDC configuration
    //const uint32_t tdc_address = 0x000d0000; // V1290N (16 ch., Louvain-la-Neuve)
    const uint32_t tdc_address_board1 = 0x00aa0000; // V1290A (32 ch., CERN)
    const uint32_t tdc_address_board2 = 0x00bb0000; // V1290A (32 ch., CERN)
    
    vme->AddTDC(tdc_address_board1);
    tdc1 = vme->GetTDC(tdc_address_board1);
    tdc1->SetVerboseLevel(0);
    //tdc1->GetControl().Dump();
    tdc1->SetAcquisitionMode(acq_mode);
    tdc1->SetDetectionMode(det_mode);
    tdc1->SetDLLClock(VME::TDCV1x90::DLL_PLL_HighRes);
    //tdc1->SetETTT();
    
    vme->AddTDC(tdc_address_board2);
    tdc2 = vme->GetTDC(tdc_address_board2);
    tdc2->SetVerboseLevel(0);
    //tdc2->GetControl().Dump();
    tdc2->SetAcquisitionMode(acq_mode);
    tdc2->SetDetectionMode(det_mode);
    tdc2->SetDLLClock(VME::TDCV1x90::DLL_PLL_HighRes);
    //tdc2->SetETTT();
    
    //filename = GenerateFileName(0);
    out_file_board1.open("events_board1.dat", fstream::out | ios::binary );	
    out_file_board2.open("events_board2.dat", fstream::out | ios::binary );	
    if (!out_file_board1.is_open() or !out_file_board2.is_open()) {
      throw Exception(__PRETTY_FUNCTION__, "Error opening files", Fatal);
    }
    
    t_beg = std::time(0);
    std::string acqmode, detmode;
    switch (tdc1->GetAcquisitionMode()) {
      case VME::CONT_STORAGE: acqmode = "Continuous storage"; break;
      case VME::TRIG_MATCH: acqmode = "Trigger matching"; break;
      default: acqmode = "[Invalid mode]"; throw Exception(__PRETTY_FUNCTION__, "Invalid acquisition mode!", Fatal);
    }
    switch (tdc1->GetDetectionMode()) {
      case VME::PAIR: detmode = "Pair measurement"; break;
      case VME::OLEADING: detmode = "Leading edge only"; break;
      case VME::OTRAILING: detmode = "Trailing edge only"; break;
      case VME::TRAILEAD: detmode = "Leading and trailing edges"; break;
    }
    cerr << endl << "*** Ready for acquisition! ***" << endl
         << "Acquisition mode: " << acqmode << endl 
         << "Detection mode: " << detmode << endl 
         << "Local time: " << asctime(std::localtime(&t_beg));
    
    out_file_board1.write((char*)&fh, sizeof(file_header_t));
    out_file_board2.write((char*)&fh, sizeof(file_header_t));
    while (true) {
      // board 1
      ec = tdc1->FetchEvents();
      if (ec.size()==0) continue; // no events were fetched
      for (VME::TDCEventCollection::const_iterator e=ec.begin(); e!=ec.end(); e++) {
        out_file_board1.write((char*)&(*e), sizeof(VME::TDCEvent));
      }
      num_events_board1 += ec.size();

      // board 2
      ec = tdc2->FetchEvents();
      if (ec.size()==0) continue; // no events were fetched
      for (VME::TDCEventCollection::const_iterator e=ec.begin(); e!=ec.end(); e++) {
        out_file_board2.write((char*)&(*e), sizeof(VME::TDCEvent));
      }
      num_events_board2 += ec.size();
    }
  } catch (Exception& e) {
    if (e.ErrorNumber()==TDC_ACQ_STOP) {
      if (out_file_board1.is_open()) out_file_board1.close();
      if (out_file_board2.is_open()) out_file_board2.close();

      std::time_t t_end = std::time(0);
      double nsec_tot = difftime(t_end, t_beg), nsec = fmod(nsec_tot,60), nmin = (nsec_tot-nsec)/60.;
      cerr << endl << "*** Acquisition stopped! ***" << endl
           << "Local time: " << asctime(std::localtime(&t_end))
           << "Total acquisition time: " << difftime(t_end, t_beg) << " seconds"
           << " (" << nmin << " min " << nsec << " sec)"
           << endl;
    
      cerr << endl << "Acquired " << num_events_board1 << " / " << num_events_board2 << " words in this run" << endl;
  
      delete vme;
      return 0;
    }
    e.Dump();
    return -1;
  }
    
  return 0;
}
