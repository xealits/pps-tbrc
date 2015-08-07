#include <iostream>

#include "FileReader.h"

using namespace std;
using namespace VME;

int
main(int argc, char* argv[])
{
  unsigned int channel_id = 0;
  if (argc<2) {
    cerr << "Usage:\n\t" << argv[0] << " <input file>" << endl;
    return -1;
  }
  if (argc>=3) {
    channel_id = atoi(argv[2]);
  }

  TDCMeasurement m;
  TDCEvent e;
  FileReader f(argv[1]);
  cout << "Run/burst id: " << f.GetRunId() << " / " << f.GetBurstId() << endl;
  cout << "Acquisition mode: " << f.GetAcquisitionMode() << endl;
  cout << "Detection mode: " << f.GetDetectionMode() << endl;
  while (true) {
    try {
      if (!f.GetNextEvent(&e)) break;
      e.Dump();
    } catch (Exception& e) { e.Dump(); }
  }
 
  return 0;
}
