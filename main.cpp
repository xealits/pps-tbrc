//#include "Exception.h"
#include "Messenger.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
  Messenger* m = new Messenger(1987);

  if (!m->Connect()) {
    cout << "Error while trying to connect the messenger!" << endl;
    return -1;
  }

  while (true) {
    m->Receive();
  }

  delete m;
  return 0;
}
