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
  int num_events;
  TH1D* hist_tot = new TH1D("tot", "", 100000, 0., 10000.);
  
  FileReader f(argv[1]);
  //cout << f.GetNumTDCs() << " TDCs recorded" << endl;
  num_events = 0;
  try {
    while (f.GetNextMeasurement(channel_id, &m)) {
      hist_tot->Fill(m.GetToT()*25./1024.);
      //m.Dump();
      num_events++;
    }
  } catch (Exception& e) { e.Dump(); }
  cerr << "number of events in channel " << channel_id << ": " << num_events << endl;
  
  TCanvas* c = new TCanvas;
  hist_tot->Draw();
  hist_tot->GetXaxis()->SetTitle("Time over threshold (ns)");
  hist_tot->GetYaxis()->SetTitle("Events");
  c->SaveAs("dist_tot.png");
  c->SetLogy();
  c->SaveAs("dist_tot_logscale.png");

  return 0;
}
