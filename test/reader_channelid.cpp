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
  
  const unsigned int num_channels = 32;

  TH1D* h[num_channels];

  for (unsigned int i=0; i<num_channels; i++) {
    h[i] = new TH1D(Form("tot_%i",i), "", 100, 0., 200.);
    FileReader f(argv[1]);
    num_events = 0;
    while (true) {
      try {
        if (!f.GetNextMeasurement(i, &m)) break;
        //cout << m.GetLeadingTime()-m.GetTrailingTime() << endl;
        for (unsigned int j=0; j<m.NumEvents(); j++) {
          h[i]->Fill(m.GetToT(j)*25./1024.);
        }
        num_events += m.NumEvents();
      } catch (Exception& e) { /*e.Dump();*/ }
    }
    cout << "number of events in channel " << i << ": " << num_events << endl;
  }

  gStyle->SetPadGridX(true); gStyle->SetPadGridY(true);
  gStyle->SetPadTickX(true); gStyle->SetPadTickY(true);
  gStyle->SetOptStat(0);

  TCanvas* c = new TCanvas("canv_multichannels", "", 1200, 800);
  c->Divide(2);
  TLegend *leg1, *leg2;
  leg1 = new TLegend(0.12, 0.65, 0.6, 0.88);
  leg2 = new TLegend(0.12, 0.65, 0.6, 0.88);
  double mx1 = -1., mx2 = -1;
  c->cd(1);
  for (unsigned int i=0; i<num_channels; i++) {
    if (i==num_channels/2) {
      leg1->Draw();
      c->cd(2);
    }
    if (i==0 or i==num_channels/2) h[i]->Draw();
    else                           h[i]->Draw("same");
    if (i<num_channels/2) {
      h[i]->SetLineColor(i+2);// h[i]->SetLineStyle(1);
      mx1 = max(mx1, h[i]->GetMaximum());
    }
    else {
      h[i]->SetLineColor(i+2-num_channels/2);// h[i]->SetLineStyle(2);
      mx2 = max(mx2, h[i]->GetMaximum());
    }
    if (h[i]->Integral()!=0) {
      h[i]->Fit("gaus", "0");
      TF1* f = (TF1*)h[i]->GetFunction("gaus");
      if (i<num_channels/2) leg1->AddEntry(h[i], Form("Channel %i N=%.1f, #mu=%.4g, #sigma=%.3g",i,h[i]->Integral(),f->GetParameter(1),f->GetParameter(2)), "l");
      else                  leg2->AddEntry(h[i], Form("Channel %i N=%.1f, #mu=%.4g, #sigma=%.3g",i,h[i]->Integral(),f->GetParameter(1),f->GetParameter(2)), "l");
    }
    else {
      if (i<num_channels/2) leg1->AddEntry(h[i], Form("Channel %i",i), "l");
      else                  leg2->AddEntry(h[i], Form("Channel %i",i), "l");
    }
  }
  leg2->Draw();
  h[0]->GetXaxis()->SetTitle("Time over threshold (ns)");
  h[0]->GetYaxis()->SetTitle("Events");
  h[0]->GetYaxis()->SetRangeUser(.1, mx1*1.3);
  h[num_channels/2]->GetXaxis()->SetTitle("Time over threshold (ns)");
  h[num_channels/2]->GetYaxis()->SetTitle("Events");
  h[num_channels/2]->GetYaxis()->SetRangeUser(.1, mx2*1.3);
  leg1->SetTextFont(43); leg1->SetTextSize(14);
  leg2->SetTextFont(43); leg2->SetTextSize(14);
  c->SaveAs("dist_tot_multichannels.png");
  c->SaveAs("dist_tot_multichannels.pdf");
  
  return 0;
}
