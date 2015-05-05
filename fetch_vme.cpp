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
  if (gEnd==0) {
    cout << endl << "[C-c] Trying a clean exit!" << endl;
    out_file.close();
    vme->GetTDC()->abort();
  }
  else if (gEnd>=5) {
    cout << endl << "[C-c > 5 times] ... Forcing exit!" << endl;
    exit(0);
  }
  gEnd++;
}

int main(int argc, char *argv[]) {
  
  unsigned int num_events;
  VME::TDCEventCollection ec;
  string filename;
  
  signal(SIGINT, CtrlC);
  
  file_header_t fh;
  fh.magic = 0x30535050; // PPS0 in ASCII
  fh.run_id = 0;
  fh.spill_id = 0;
  
  try {
    vme = new VMEReader("/dev/usb/v1718_0", 1718);
    
    // TDC configuration
    vme->GetTDC()->SetWindowWidth(2040);
    vme->GetTDC()->SetWindowOffset(-2045);
    vme->GetTDC()->WaitMicro(VME::WRITE_OK);
    
    filename = GenerateFileName(0);
    out_file.open(filename.c_str(), fstream::out | ios::binary );	
    if (!out_file.is_open()) {
      ostringstream o; o << "Error opening file " << filename;
      throw Exception(__PRETTY_FUNCTION__, o.str(), Fatal);
    }
    
    fh.run_id = vme->GetRunNumber();
    
    cout << endl << "*** Ready for acquisition! ***" << endl;
    
    num_events = 0;
    out_file.write((char*)&fh, sizeof(file_header_t));
    while (true) {
      ec = vme->GetTDC()->FetchEvents();
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
