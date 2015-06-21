#include <iostream>

#include "FileReader.h"

using namespace std;
using namespace VME;

int
main(int argc, char* argv[])
{
  if (argc<2) {
    cout << "Usage:\n\t" << argv[0] << " <input file>" << endl;
    return -1;
  }
  
  FileReader f(argv[1], VME::CONT_STORAGE);
  cout << f.GetNumTDCs() << " TDCs recorded" << endl;

  TDCMeasurement m;
  while (f.GetNextMeasurement(0, &m)) {
    //m.Dump();
    cout << m.GetLeadingTime()-m.GetTrailingTime() << endl;
  }
  
  return 0;
}
