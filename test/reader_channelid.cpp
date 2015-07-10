#include <iostream>

#include "FileReader.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TStyle.h"
#include "TLegend.h"

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
  
  const unsigned int num_channels = 16;

  TH1D* h[num_channels];

  for (unsigned int i=0; i<num_channels; i++) {
    h[i] = new TH1D(Form("tot_%i",i), "", 150, 22., 25.);
    FileReader f(argv[1]);
    num_events = 0;
    while (true) {
      try {
        if (!f.GetNextMeasurement(i, &m)) break;
        //cout << m.GetLeadingTime()-m.GetTrailingTime() << endl;
        for (unsigned int j=0; j<m.NumEvents(); j++) {
          //std::cout << "--> " << (m.GetToT(i)*25./1024.) << std::endl;
          h[i]->Fill(m.GetToT(j)*25./1024.);
        }
        num_events += m.NumEvents();
      } catch (Exception& e) { e.Dump(); }
    }
    cout << "number of events in channel " << i << ": " << num_events << endl;
  }

  gStyle->SetPadGridX(true); gStyle->SetPadGridY(true);
  gStyle->SetPadTickX(true); gStyle->SetPadTickY(true);
  gStyle->SetOptStat(0);

  TCanvas* c = new TCanvas("canv_multichannels", "", 1200, 800);
  c->Divide(2);
  TLegend *leg1, *leg2;
  leg1 = new TLegend(0.15, 0.6, 0.5, 0.88);
  leg2 = new TLegend(0.15, 0.6, 0.5, 0.88);
  double mx = -1.;
  c->cd(1);
  for (unsigned int i=0; i<num_channels; i++) {
    if (i==num_channels/2) {
      leg1->Draw();
      c->cd(2);
    }
    if (i==0 or i==num_channels/2) h[i]->Draw();
    else                           h[i]->Draw("same");
    mx = max(mx, h[i]->GetMaximum());
    if (i<num_channels/2) {
      h[i]->SetLineColor(i+2);// h[i]->SetLineStyle(1);
    }
    else {
      h[i]->SetLineColor(i+2-num_channels/2);// h[i]->SetLineStyle(2);
    }
    if (h[i]->Integral()!=0) {
      h[i]->Fit("gaus", "0");
      TF1* f = (TF1*)h[i]->GetFunction("gaus");
      if (i<num_channels/2) leg1->AddEntry(h[i], Form("Channel %i  #mu=%.3g, #sigma=%.3g",i,f->GetParameter(1),f->GetParameter(2)), "l");
      else                  leg2->AddEntry(h[i], Form("Channel %i  #mu=%.3g, #sigma=%.3g",i,f->GetParameter(1),f->GetParameter(2)), "l");
    }
    else {
      if (i<num_channels/2) leg1->AddEntry(h[i], Form("Channel %i",i), "l");
      else                  leg2->AddEntry(h[i], Form("Channel %i",i), "l");
    }
  }
  leg2->Draw();
  h[0]->GetYaxis()->SetRangeUser(.1, mx*1.3);
  h[0]->GetXaxis()->SetTitle("Time over threshold (ns)");
  h[0]->GetYaxis()->SetTitle("Events");
  h[num_channels/2]->GetYaxis()->SetRangeUser(.1, mx*1.2);
  h[num_channels/2]->GetXaxis()->SetTitle("Time over threshold (ns)");
  h[num_channels/2]->GetYaxis()->SetTitle("Events");
  c->SaveAs("tot_multichannels.png");
  
  return 0;
}
