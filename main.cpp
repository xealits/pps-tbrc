#include "Messenger.h"

#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
  Messenger m(1987);

  if (!m.Connect()) {
    cout << "Error while trying to connect the messenger!" << endl;
    return -1;
  }

  while (true) {
    // This part looks at new clients connections
    Messenger mm;
    m.AcceptConnections(mm);
    //while (true) {
    mm.Receive();
    //}
    
    mm.Broadcast("prout");
    //sleep(1);

  }

  return 0;
}
