#include "VMEReader.h"
#include "FileConstants.h"

#include <iostream>
#include <fstream>
#include <string>
#include <signal.h>

using namespace std;

VMEReader* vme;
fstream out_file;
int gEnd = 0;

void CtrlC(int aSig) {
  if (gEnd==0) { cout << endl << "[C-c] Trying a clean exit!" << endl;
    out_file.close();
    vme->Abort();
  }
  else if (gEnd>=5) { cout << endl << "[C-c > 5 times] ... Forcing exit!" << endl;
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
  
  file_header_t fh;
  fh.magic = 0x30535050; // PPS0 in ASCII
  fh.run_id = 0;
  fh.spill_id = 0;
  
  try {
    bool with_socket = false;
    vme = new VMEReader("/dev/usb/v1718_0", VME::CAEN_V1718, with_socket);
    
    fh.run_id = vme->GetRunNumber();
    
    // TDC configuration
    vme->AddTDC(0x000d0000);
    tdc = vme->GetTDC(0x000d0000);
    tdc->SetWindowWidth(2040);
    tdc->SetWindowOffset(-2045);
    tdc->WaitMicro(VME::WRITE_OK);
    
    filename = GenerateFileName(0);
    out_file.open(filename.c_str(), fstream::out | ios::binary );	
    if (!out_file.is_open()) {
      ostringstream o; o << "Error opening file " << filename;
      throw Exception(__PRETTY_FUNCTION__, o.str(), Fatal);
    }
    
    cout << endl << "*** Ready for acquisition! ***" << endl;
    
    num_events = 0;
    out_file.write((char*)&fh, sizeof(file_header_t));
    while (true) {
      ec = tdc->FetchEvents();
      if (ec.size()==0) continue; // no events were fetched
      for (VME::TDCEventCollection::const_iterator e=ec.begin(); e!=ec.end(); e++) {
        out_file.write((char*)&(*e), sizeof(VME::TDCEvent));
      }
      num_events += ec.size();
    }
    out_file.close();
    
    cout << "Acquired " << num_events << " in this run" << endl;
  
    delete vme;
  } catch (Exception& e) {
    e.Dump();
    return -1;
  }
    
  return 0;
}
