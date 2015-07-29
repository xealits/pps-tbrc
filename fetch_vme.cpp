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
  if (gEnd==0) { cerr << endl << "[C-c] Trying a clean exit!" << endl; vme->Abort(); }
  else if (gEnd>=5) { cerr << endl << "[C-c > 5 times] ... Forcing exit!" << endl; exit(0); }
  gEnd++;
}

int main(int argc, char *argv[]) {
  signal(SIGINT, CtrlC);
  
  try {
    bool with_socket = false;
    vme = new VMEReader("/dev/usb/v2718_0", VME::CAEN_V2718, with_socket);

    vme->AddCFD(0x070000);
    VME::CFDV812* cfd = vme->GetCFD(0x070000);
    cout << cfd->GetSerialNumber() << endl;
    cfd->SetPOI(0xf);
    for (unsigned int i=0; i<16; i++) cfd->SetThreshold(i, 0x1);
 
    vme->AddHVModule(0x900000, 0xa);
    NIM::HVModuleN470* hv = vme->GetHVModule();
    cout << "module id=" << hv->GetModuleId() << endl;
    hv->ReadMonitoringValues();
    hv->SetChannelV0(0, 320);
    hv->SetChannelI0(0, 0);
    hv->ReadChannelValues(0);
    
  } catch (Exception& e) {
    e.Dump();
    return -1;
  }
    
  return 0;
}
