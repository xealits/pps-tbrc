#include <iostream>

#include "FileReader.h"

#include "TCanvas.h"
#include "TH1.h"

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
  unsigned int num_events, num_triggers;
  TH1D* hist_tot = new TH1D("tot", "", 800, 450., 470.);
  
  FileReader f(argv[1]);
  //cout << f.GetNumTDCs() << " TDCs recorded" << endl;
  num_triggers = num_events = 0;
  try {
    while (f.GetNextMeasurement(channel_id, &m)) {
      //m.Dump();
      for (unsigned int i=0; i<m.NumEvents(); i++) {
        std::cout << "--> " << (m.GetToT(i)*25./1024.) << std::endl;
        hist_tot->Fill(m.GetToT(i)*25./1024.);
      }
      num_events += m.NumEvents();
      num_triggers += 1;
    }
  } catch (Exception& e) { e.Dump(); }
  cerr << "total number of triggers: " << num_triggers << endl;
  cerr << "mean number of events per trigger in channel " << channel_id << ": " << ((float)num_events/num_triggers) << endl;
  
  TCanvas* c = new TCanvas;
  hist_tot->Draw();
  hist_tot->GetXaxis()->SetTitle("Time over threshold (ns)");
  hist_tot->GetYaxis()->SetTitle("Events");
  c->SaveAs("dist_tot.png");
  c->SetLogy();
  c->SaveAs("dist_tot_logscale.png");

  return 0;
}
