#include <iostream>

#include "FileReader.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TLegend.h"
#include "TStyle.h"

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

  TDCMeasurement m;
  unsigned int num_triggers;
  TH1D* hist_numerrors = new TH1D("nerrors", "", 10, -.5, 9.5);
  
  FileReader f(argv[1]);
  //cout << f.GetNumTDCs() << " TDCs recorded" << endl;
  num_triggers = 0;
  while (true) {
    try {
      if (!f.GetNextMeasurement(channel_id, &m)) break;
      hist_numerrors->Fill(m.NumErrors()-.5);
      if (m.NumErrors()) cout << m.NumErrors() << endl;
      //cerr << "ettt=" << m.GetETTT() << endl;
      num_triggers += 1;
    } catch (Exception& e) { e.Dump(); }
  }
  cerr << "total number of triggers: " << num_triggers << endl;
 
  gStyle->SetPadGridX(true); gStyle->SetPadGridY(true);
  gStyle->SetPadTickX(true); gStyle->SetPadTickY(true);
 
  TCanvas* c_nerrors = new TCanvas;
  hist_numerrors->Draw();
  hist_numerrors->GetXaxis()->SetTitle("Number of errors / trigger");
  hist_numerrors->GetYaxis()->SetTitle("Triggers");
  c_nerrors->SaveAs("dist_nerrors.png");

  return 0;
}
