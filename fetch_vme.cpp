#include "VMEBridgeV1718.h"
#include "VMETDCV1x90.h"
#include "TDCEvent.h"

#include <iostream>
#include <fstream>
#include <string>
#include <signal.h>

using namespace std;

struct file_header_t {
  uint32_t magic;
  uint32_t run_id;
  uint32_t spill_id;
};

VMEBridgeV1718 *bridge;
VMETDCV1x90* tdc;
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
  
  if(argc != 2) {
    std::cout << "Usage: " << argv[0] << " FILENAME" << std::endl;
    exit(-1);
  }
  //FIXME: Checks on the filename !!!
  
  int32_t bhandle;
  bridge = new VMEBridgeV1718("/dev/usb/v1718_0");
  bhandle = bridge->getBHandle();
 
  signal(SIGINT, CtrlC);
  
  try {
    tdc = new VMETDCV1x90(bhandle,0x000d0000,TRIG_MATCH,TRAILEAD);
    tdc->GetFirmwareRev();
    
    // TDC configuration
    tdc->SetWindowWidth(2040);
    tdc->SetWindowOffset(-2045);
    
    tdc->WaitMicro(WRITE_OK);
    //tdc->softwareClear(); //FIXME don't forget to erase
    //std::cout << "Are header and trailer bytes sent in BLT? " << tdc->getTDCEncapsulation() << std::endl;
    //FIXME: Need to check the user input
   	out_file.open(argv[1],std::fstream::out | std::ios::binary );	
    if (!out_file.is_open()) {
      std::cerr << argv[0] << ": error opening file " << argv[1] << std::endl;
      return -1;
    }
    file_header_t fh;
    fh.magic = 0x47544B30; //ASCII: GTK0 
    fh.run_id = 0;
    fh.spill_id = 0;
    
    std::cout << std::endl << "*** Ready for acquisition! ***" << std::endl;
    
    out_file.write((char*)&fh, sizeof(file_header_t));
    while (true) {
      TDCEventCollection ec = tdc->GetEvents();
      if (ec.size()==0) continue;
      for (TDCEventCollection::const_iterator e=ec.begin(); e!=ec.end(); e++) {
        out_file.write((char*)&(*e), sizeof(TDCEvent));
      }
    }
    
    out_file.close();
  
    delete tdc;
  } catch (Exception& e) {
    e.Dump();
  }
  
  
  delete bridge;
  return 0;
}
