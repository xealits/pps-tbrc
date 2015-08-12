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

    try { vme->AddHVModule(0x500000, 0xc); } catch (Exception& e) { e.Dump(); }
    NIM::HVModuleN470* hv = vme->GetHVModule();
    cout << "module id=" << hv->GetModuleId() << endl;
    //hv->ReadMonitoringValues().Dump();
    hv->SetChannelV0(0, 2400);
    //hv->SetChannelI0(0, 500);
    hv->ReadChannelValues(0).Dump();
    hv->ReadChannelValues(3).Dump();
    
  } catch (Exception& e) {
    e.Dump();
    return -1;
  }
    
  return 0;
}
