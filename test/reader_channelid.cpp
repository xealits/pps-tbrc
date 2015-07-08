#include <iostream>

#include "FileReader.h"
#include "TCanvas.h"
#include "TH1.h"

using namespace std;
using namespace VME;

int
main(int argc, char* argv[])
{
  if (argc<2) {
    cout << "Usage:\n\t" << argv[0] << " <input file>" << endl;
    return -1;
  }
  TDCMeasurement m;
  int num_events;
  
  const unsigned int num_channels = 3;

  TH1D* h[num_channels];

  for (unsigned int i=0; i<num_channels; i++) {
    h[i] = new TH1D("tot____"+i, "", 800, 450., 470.);
    FileReader f(argv[1]);
    num_events = 0;
    while (f.GetNextMeasurement(i, &m)) {
      //cout << m.GetLeadingTime()-m.GetTrailingTime() << endl;
      for (unsigned int j=0; j<m.NumEvents(); j++) {
        //std::cout << "--> " << (m.GetToT(i)*25./1024.) << std::endl;
        h[i]->Fill(m.GetToT(j)*25./1024.);
      }
      num_events++;
    }
    cout << "number of events in channel " << i << ": " << num_events << endl;
  }

  TCanvas* c = new TCanvas;
  for (unsigned int i=0; i<num_channels; i++) {
    if (i==0) h[i]->Draw();
    else      h[i]->Draw("same");
    h[i]->SetLineColor(i);
    h[i]->SetLineStyle(i);
  }
  c->SaveAs("tot_multichannels.png");
  
  return 0;
}
