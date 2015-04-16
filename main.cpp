#include "Messenger.h"

#include <iostream>
#include <unistd.h>

using namespace std;

Messenger* m = 0;
int gEnd = 0;

void CtrlC(int aSig) {
  if (gEnd==0) {
    cout << endl << "[C-c] Trying a clean exit!" << endl;
    if (m) { cout << "Trying to disconnect the messenger" << endl; delete m; }
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
  
  pid_t pid;
  

  while (true) {
    try {
      m->Receive();
    } catch (Exception& e) {
      e.Dump();
    }
  }
  
  delete m;
  return 0;
}
