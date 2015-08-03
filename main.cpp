#include "Messenger.h"
#include "Client.h"
#include "FileConstants.h"

#include <iostream>
#include <fstream>

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

  // Where to put the logs
  ofstream err_log("master.err", ios::binary);
  const Logger lr(err_log, cerr);

  m = new Messenger(1987);
  if (!m->Connect()) {
    cout << "Failed to connect the messenger!" << endl;
    return -1;
  }
  while (true) {
    try { m->Receive(); } catch (Exception& e) { e.Dump(); }
  }

  delete m;
  return 0;
}
