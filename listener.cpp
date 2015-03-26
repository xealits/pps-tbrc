#include "Listener.h"

#include <cstdlib>
#include <iostream>

using namespace std;

Listener* l;
int gEnd=0;

void CtrlC(int aSig) {
  if (gEnd==0) {
    cout << endl << "Ctrl-C detected... trying clean exit!" << endl;
    l->Disconnect();
  }
  else if (gEnd>=5) {
    cout << endl << "Ctrl-C detected more than five times... forcing exit!" << endl;
    exit(0);
  }
  gEnd++;
}

int main(int argc, char* argv[])
{
  if (argc<2) return -1;
  signal(SIGINT, CtrlC);
  
  const int port = atoi(argv[1]);
  cout << "Starting to listen on port " << port << endl;

  l = new Listener(port);
  if (!l->Connect()) {
    cout << "Failed to connect the listener" << endl;
    return -1;
  }

  /*while (true) {
    Listener ll;
    try {
      l.AcceptConnections(ll);
      ll.Receive();
    } catch (Exception& e) {
      e.Dump();
    }
  }*/

  delete l;
  return 0;
}
