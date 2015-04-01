#include "Messenger.h"

#include <iostream>

using namespace std;

Messenger* m = 0;
int gEnd = 0;

void CtrlC(int aSig) {
  if (gEnd==0) {
    cout << endl << "[C-c] Trying a clean exit!" << endl;
    if (m) {
      cout << "Trying to disconnect the messenger" << endl;
      m->Disconnect();
      delete m;
    }
    exit(0);
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
  m = new Messenger(1987);

  if (!m->Connect()) {
    cout << "Error while trying to connect the messenger!" << endl;
    return -1;
  }

  /*while (true) {
    Messenger mm(m->GetListeners());
    // This part looks at new clients connections
    m->AcceptConnections(mm);
    while (true) {
      try {
        mm.Receive();
      } catch (Exception& e) {
        std::cout << "prout" << std::endl;
        e.Dump();
        break;
      }
    }
    m->AddListeners(mm.GetListeners());
    //mm.DumpListeners();    
    //mm.Broadcast("prout");
    //sleep(1);
    //mm.Disconnect();
    //delete mm;
  } */
  while (true) {
    try {
      m->Receive();
    } catch (Exception& e) {
      e.Dump();
      break;
    }
  }
  
  delete m;
  return 0;
}
