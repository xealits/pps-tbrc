#include "Listener.h"

#include <cstdlib>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
  if (argc<2) return -1;
  const int port = atoi(argv[1]);
  cout << "Starting to listen on port " << port << endl;

  Listener* l = new Listener(port);
  if (!l->Connect()) {
    return -1;
  }

  while (true) {
    //cout << "Listening" << endl;
  }

  delete l;
  return 0;
}
