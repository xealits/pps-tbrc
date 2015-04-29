#include <iostream>

#include "FileReader.h"

using namespace std;

int
main(int argc, char* argv[])
{
  if (argc<2) {
    cout << "Usage:\n\t" << argv[0] << " <input file>" << std::endl;
    return -1;
  }
  
  FileReader f(argv[1]);
  std::cout << f.GetNumTDCs() << " TDCs recorded" << std::endl;
  
  return 0;
}
