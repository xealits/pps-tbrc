#include "OnlineDBHandler.h"

using namespace std;

int
main(int argc, char* argv[])
{
  try {
    OnlineDBHandler run("test.db");
    //run.NewRun();
    run.NewBurst();
    cout << run.GetLastRun() << endl;
    cout << run.GetLastBurst(run.GetLastRun()) << endl;
    OnlineDBHandler::BurstInfos bi = run.GetRunInfo(run.GetLastRun());
    for (OnlineDBHandler::BurstInfos::iterator it=bi.begin(); it!=bi.end(); it++) {
      cout << " spill id " << it->first << " began at " << it->second << endl;
    }
  } catch (Exception& e) {
    e.Dump();
  }
  return 0;
}
