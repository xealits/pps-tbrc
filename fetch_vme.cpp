#include "VME_BridgeVx718.h"
#include "VME_TDCV1x90.h"
#include "TDCEvent.h"
#include "FileConstants.h"

#include <iostream>
#include <fstream>
#include <string>
#include <signal.h>

using namespace std;

VME::BridgeVx718 *bridge;
VME::TDCV1x90* tdc;
std::fstream out_file;
int gEnd = 0;

void CtrlC(int aSig) {
  if (gEnd==0) {
    cout << endl << "[C-c] Trying a clean exit!" << endl;
    out_file.close();
    tdc->abort();
  }
  else if (gEnd>=5) {
    cout << endl << "[C-c > 5 times] ... Forcing exit!" << endl;
    exit(0);
  }
  gEnd++;
}

int main(int argc, char *argv[]) {
    
  int32_t bhandle;
  unsigned int num_events;
  signal(SIGINT, CtrlC);

  try {
    bridge = new VME::BridgeVx718("/dev/usb/v1718_0", 1718);
    bhandle = bridge->GetHandle();
     
    tdc = new VME::TDCV1x90(bhandle,0x000d0000, VME::TRIG_MATCH, VME::TRAILEAD);
    tdc->GetFirmwareRev();
    
    // TDC configuration
    tdc->SetWindowWidth(2040);
    tdc->SetWindowOffset(-2045);
    
    tdc->WaitMicro(VME::WRITE_OK);
    //tdc->SoftwareClear(); //FIXME don't forget to erase
    //std::cout << "Are header and trailer bytes sent in BLT? " << tdc->GetTDCEncapsulation() << std::endl;
    //FIXME: Need to check the user input
    std::string filename = GenerateFileName(0);
    out_file.open(filename.c_str(), std::fstream::out | std::ios::binary );	
    if (!out_file.is_open()) {
      std::cerr << argv[0] << ": error opening file " << argv[1] << std::endl;
      return -1;
    }
    file_header_t fh;
    fh.magic = 0x47544B30; //ASCII: GTK0 
    fh.run_id = 0;
    fh.spill_id = 0;
    
    std::cout << std::endl << "*** Ready for acquisition! ***" << std::endl;
    
    num_events = 0;
    out_file.write((char*)&fh, sizeof(file_header_t));
    while (true) {
      TDCEventCollection ec = tdc->GetEvents();
      if (ec.size()==0) continue;
      for (TDCEventCollection::const_iterator e=ec.begin(); e!=ec.end(); e++) {
        out_file.write((char*)&(*e), sizeof(TDCEvent));
      }
      num_events += ec.size();
    }
    out_file.close();
    
    std::cout << "Acquired " << num_events << " in this run" << std::endl;
  
    delete tdc;
    delete bridge;
  } catch (Exception& e) {
    e.Dump();
    return -1;
  }
    
  return 0;
}
