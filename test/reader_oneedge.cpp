#include <iostream>

#include "FileReader.h"

using namespace std;
using namespace VME;

int
main(int argc, char* argv[])
{
  unsigned int channel_id = 0;
  if (argc<2) {
    cout << "Usage:\n\t" << argv[0] << " <input file>" << endl;
    return -1;
  }
  if (argc>=3) {
    channel_id = atoi(argv[2]);
  }

  TDCMeasurement m;
  int num_events;
  
  FileReader f(argv[1]);
  //cout << f.GetNumTDCs() << " TDCs recorded" << endl;
  num_events = 0;
  int last_value = -1;
  try {
    while (f.GetNextMeasurement(channel_id, &m)) {
      if (last_value<0) { last_value = m.GetLeadingTime(); continue; }
      
      cout << m.GetLeadingTime() << "\t" << m.GetLeadingTime()-last_value << endl;
      last_value = m.GetLeadingTime();

      num_events++;
    }
  } catch (Exception& e) { e.Dump(); }
  cerr << "number of events in channel " << channel_id << ": " << num_events << endl;
  
  return 0;
}
