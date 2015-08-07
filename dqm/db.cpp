#include "DQMProcess.h"
#include "OnlineDBHandler.h"
#include "PPSCanvas.h"

#include <fstream>

#include "TGraph.h"
#include "TH1.h"

using namespace std;

bool
DBDQM(vector<string>* outputs)
{
  try {
    OnlineDBHandler ri;
    unsigned int last_run = ri.GetLastRun();
    OnlineDBHandler::BurstInfos si = ri.GetRunInfo(last_run);

    TGraph* gr_spills = new TGraph;
    TH1D* h_dist_time = new TH1D("dist_timew", "", 100, 0., 100.);
    unsigned int i = 0;
    int last_time_start = -1;
    for (OnlineDBHandler::BurstInfos::iterator s=si.begin(); s!=si.end(); s++, i++) {
      if (last_time_start>0) {
        gr_spills->SetPoint(i, s->burst_id, s->time_start-last_time_start);
        h_dist_time->Fill(s->time_start-last_time_start);
      }
      last_time_start = s->time_start;
    }
    DQM::PPSCanvas* c_spills = new DQM::PPSCanvas(Form("db_time_btw_write_%d", last_run), "Time between file writings");
    c_spills->SetRunInfo(last_run, TDatime().AsString());
    c_spills->Grid()->SetGrid(1, 1);
    gr_spills->Draw("ap");
    gr_spills->GetXaxis()->SetTitle("Burst train");
    gr_spills->GetYaxis()->SetTitle("Time to write file (s)");
    gr_spills->SetMarkerStyle(20);
    gr_spills->SetMarkerSize(.6);
    c_spills->Save("png", DQM_OUTPUT_DIR);
    outputs->push_back(c_spills->GetName());

    DQM::PPSCanvas* c_dst = new DQM::PPSCanvas(Form("db_time_btw_write_dist_%d", last_run), "Time between file writings");
    c_dst->SetRunInfo(last_run, TDatime().AsString());
    c_dst->Grid()->SetGrid(1, 1);
    h_dist_time->Draw();
    h_dist_time->GetXaxis()->SetTitle("Time to write file (s)");
    h_dist_time->GetYaxis()->SetTitle("Burst trains");
    c_dst->Save("png", DQM_OUTPUT_DIR);
    outputs->push_back(c_dst->GetName());
    
    /*delete gr_spills; delete c_spills;
    delete h_dist_time; delete c_dst;*/

  } catch (Exception& e) { e.Dump(); }
  return true;
}

int
main(int argc, char* argv[])
{
  DQM::DQMProcess dqm(1987, 4);
  dqm.Run(DBDQM, DQM::DQMProcess::UpdatedPlot);
  return 0;
}

