#include "FPGAHandler.h"
#include "TDCEvent.h"

#include <iostream>

using namespace std;

FPGAHandler* h = 0;
int gEnd = 0;

void CtrlC(int aSig) {
  if (gEnd==0) {
    cout << endl << "[C-c] Trying a clean exit!" << endl;
    if (h) delete h; exit(0);
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
  
  TDCConfiguration config;
  config.SetChannelOffset(0, 0x155);
  config.SetLeadingMode();
  config.SetTrailingMode();
  config.SetEdgeResolution(TDCConfiguration::E_6250PS);
  config.Dump();
  cout << "channel offset=0x" << hex << config.GetChannelOffset(0) << dec << endl;
  cout << "edge resolution=" << config.GetEdgeResolution() << endl;
  
  TDCEvent ev(0x4100000f);
  cout << "event: TDC id=" << ev.GetTDCId() << endl;
  cout << "event: leading edge? " << (ev.GetType()==TDCEvent::LeadingEdge) << endl;
  cout << "event: trailing edge? " << (ev.GetType()==TDCEvent::TrailingEdge) << endl;
  
  try {
    h->Connect();
    h->SendConfiguration(config);
  } catch (Exception& e) {
    e.Dump();
  }

  cout << " --> Output filename: " << h->GetFilename() << endl;
  cout << endl << "*** Ready for acquisition! ***" << endl << endl;

  /*while (true) {
    try {
      h->Send(SocketMessage(GET_CLIENTS));
      h->Receive();
      sleep(2);
    } catch (Exception& e) {
      e.Dump();
      //exit(0);
    }
  }*/

  delete h;
  return 0;
}
