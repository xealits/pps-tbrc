#include "Messenger.h"
#include "Client.h"

#include <iostream>

using namespace std;

Messenger* m = 0;
Client* l = 0;
int gEnd = 0;

void CtrlC(int aSig) {
  if (gEnd==0) {
    cout << endl << "[C-c] Trying a clean exit!" << endl;
    if (m) { cout << "Trying to disconnect the messenger" << endl; delete m; }
    if (l) { cout << "Trying to disconnect the first client" << endl; delete l; }
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
  //pid_t fid = fork();
  
  //if (fid!=0) {
    m = new Messenger(1987);

    if (!m->Connect()) {
      cout << "Failed to connect the messenger!" << endl;
      return -1;
    }
    
    while (true) {
      try { m->Receive(); } catch (Exception& e) { e.Dump(); }
    }
    
    delete m;
  /*}
  else { // We're in the child
    sleep(1);
    l = new Client(1987);
    if (!l->Connect()) {
      cout << "Failed to connect the listener!" << endl;
      return -1;
    }
    while (true) {
      try { l->Receive(); } catch (Exception& e) { e.Dump(); }
    }
  }*/  
  
  return 0;
}
