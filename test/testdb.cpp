#include "OnlineDBHandler.h"

using namespace std;

int
main(int argc, char* argv[])
{
  try {
    OnlineDBHandler run("test.db");
    /*for (unsigned int i=0; i<10; i++) {
      run.NewRun();
      for (unsigned int j=0; j<i; j++) run.NewBurst();
    }*/
    cout << run.GetLastRun() << endl;
    cout << run.GetLastBurst(run.GetLastRun()) << endl;
    OnlineDBHandler::BurstInfos bi = run.GetRunInfo(run.GetLastRun());
    for (OnlineDBHandler::BurstInfos::iterator it=bi.begin(); it!=bi.end(); it++) {
      cout << " spill id " << it->burst_id << " began at " << it->time_start << endl;
    }
    /*run.SetTDCConditions(0, 0x00aa0000, 0, 1, "quartic_1");
    run.SetTDCConditions(1, 0x00bb0000, 0, 2, "quartic_2");*/
  } catch (Exception& e) {
    e.Dump();
  }
  return 0;
}
