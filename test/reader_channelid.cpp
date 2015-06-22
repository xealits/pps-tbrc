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
  TDCMeasurement m;
  int num_events;
  
  for (int channel=0; channel<15; channel++) {
  FileReader f(argv[1], VME::CONT_STORAGE);
  //cout << f.GetNumTDCs() << " TDCs recorded" << endl;
  num_events = 0;
  while (f.GetNextMeasurement(channel, &m)) {
    //cout << m.GetLeadingTime()-m.GetTrailingTime() << endl;
    num_events++;
  }
  cout << "number of events in channel " << channel << ": " << num_events << endl;
  }
  
  return 0;
}
