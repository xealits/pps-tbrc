#include "FPGAHandler.h"
#include "TDCEvent.h"

#include <iostream>

using namespace std;

FPGAHandler* h = 0;
int gEnd = 0;

void CtrlC(int aSig) {
  if (gEnd==0) {
    cout << endl << "[C-c] Trying a clean exit!" << endl;
    if (h) h->Stop();
  }
  else if (gEnd>=5) {
    cout << endl << "[C-c > 5 times] ... Forcing exit!" << endl;
    exit(0);
  }
  gEnd++;
}

int main(int argc, char* argv[])
{
  signal(SIGINT, CtrlC);

  h = new FPGAHandler(1987, "/dev/usbmon");

  try {
    h->Connect();
    //h->SetTDCSetup(config);
    h->OpenFile();
  } catch (Exception& e) {
    e.Dump();
  }

  cout << " --> Output filename: " << h->GetFilename() << endl;
  cout << endl << "*** Ready for acquisition! ***" << endl << endl;
  
  int total_nevts = 0, nevts;
  while (true) {
    if (!h) return -1;
    nevts = h->ReadBuffer();
    if (nevts<0) break;
    total_nevts += nevts;
  }
  cout << "Number of events collected: " << total_nevts << endl;
  
  delete h;
  return 0;
}
