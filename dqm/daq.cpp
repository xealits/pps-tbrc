#include "DQMProcess.h"
#include "RunFile.h"
#include "PPSCanvas.h"

#include <fstream>

#include "TGraph.h"
#include "TAxis.h"

using namespace std;

bool
DAQDQM(vector<string>* outputs)
{
  try {
    RunFileHandler ri("run_info.dat");
    unsigned int last_run = ri.GetLastRun();
    RunFileHandler::SpillInfos si = ri.GetRunInfo(last_run);

    DQM::PPSCanvas* c_spills = new DQM::PPSCanvas("daq_spills_time", "Spills arrival time");
    c_spills->SetRunInfo(last_run, "");

    TGraph* gr_spills = new TGraph;
    unsigned int i = 0;
    for (RunFileHandler::SpillInfos::iterator s=si.begin(); s!=si.end(); s++, i++) {
      gr_spills->SetPoint(i, s->second, s->first);
    }
    c_spills->Grid()->SetGrid(1, 1);
    gr_spills->Draw("alp");
    gr_spills->GetXaxis()->SetTimeDisplay(1);
    gr_spills->GetXaxis()->SetTitle("Spill arrival time");
    gr_spills->GetYaxis()->SetTitle("Spill ID");
    gr_spills->SetMarkerStyle(20);
    gr_spills->SetMarkerSize(.8);
    c_spills->Save("png", DQM_OUTPUT_DIR);
    outputs->push_back(c_spills->GetName());

  } catch (Exception& e) { e.Dump(); }
  return true;
}

int
main(int argc, char* argv[])
{
  DQM::DQMProcess dqm(1987, 0);
  dqm.Run(DAQDQM, DQM::DQMProcess::UpdatedPlot);
  return 0;
}

