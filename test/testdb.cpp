#include "OnlineDBHandler.h"

using namespace std;

int
main(int argc, char* argv[])
{
  try {
    //OnlineDBHandler run("test.db");
    OnlineDBHandler run("run_infos.db"); //WARNING! change me!!!
    /*for (unsigned int i=0; i<2; i++) {
      run.NewRun();
      run.SetTDCConditions(0, 0x00aa0000, i, 1, "quartic_1");
      run.SetTDCConditions(1, 0x00bb0000, 0, i, "quartic_2");
      for (unsigned int j=0; j<i; j++) run.NewBurst();
    }*/
    cout << run.GetLastRun() << endl;
    cout << run.GetLastBurst(run.GetLastRun()) << endl;
    OnlineDBHandler::RunCollection rc = run.GetRuns();
    for (OnlineDBHandler::RunCollection::iterator it=rc.begin(); it!=rc.end(); it++) {
      cout << "Run " << it->first << " started at " << it->second << endl;
      OnlineDBHandler::BurstInfos bi = run.GetRunInfo(it->first);
      for (OnlineDBHandler::BurstInfos::iterator b=bi.begin(); b!=bi.end(); b++) {
        cout << " burst id " << b->burst_id << " began at " << b->time_start << endl;
      }
      try {
        OnlineDBHandler::TDCConditionsCollection cond = run.GetTDCConditions(it->first);
        for (OnlineDBHandler::TDCConditionsCollection::iterator tdc=cond.begin(); tdc!=cond.end(); tdc++) {
          cout << " tdc " << tdc->tdc_id << " has address 0x" << hex << tdc->tdc_address << dec
               << " and a " << tdc->detector << " on its back"
               << endl;
        }
      } catch (Exception& e) { e.Dump(); }
    }

    
  } catch (Exception& e) {
    e.Dump();
  }
  return 0;
}
