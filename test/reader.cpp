#include <iostream>

#include "FileReader.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
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
  if (argc>=3) {
    channel_id = atoi(argv[2]);
  }

  TDCMeasurement m;
  unsigned int num_events, num_triggers;
  TH1D* hist_lead = new TH1D("lead", "", 700, -14000., 14000.);
  TH1D* hist_trail = new TH1D("trail", "", 700, -14000., 14000.);
  TH1D* hist_lead_zoom = new TH1D("lead_zoom", "", 600, 255., 325.);
  TH1D* hist_trail_zoom = new TH1D("trail_zoom", "", 600, 255., 325.);
  TH1D* hist_tot = new TH1D("tot", "", 100, 27., 29.);
  TH1D* hist_numevts = new TH1D("nevts", "", 100, -.5, 99.5);
  
  FileReader f(argv[1]);
  cout << "Run/burst id: " << f.GetRunId() << " / " << f.GetBurstId() << endl;
  cout << "Acquisition mode: " << f.GetAcquisitionMode() << endl;
  cout << "Detection mode: " << f.GetDetectionMode() << endl;
  //cout << f.GetNumTDCs() << " TDCs recorded" << endl;
  num_triggers = num_events = 0;
  while (true) {
    try {
      if (!f.GetNextMeasurement(channel_id, &m)) break;
      m.Dump();
      for (unsigned int i=0; i<m.NumEvents(); i++) {
        //std::cout << "--> " << (m.GetToT(i)*25./1024.) << std::endl;
        hist_lead->Fill(m.GetLeadingTime(i)*25./1024.);
        hist_trail->Fill(m.GetTrailingTime(i)*25./1024.);
        hist_lead_zoom->Fill(m.GetLeadingTime(i)*25./1024.);
        hist_trail_zoom->Fill(m.GetTrailingTime(i)*25./1024.);
        hist_tot->Fill(m.GetToT(i)*25./1024.);
        //std::cout << "ettt=" << m.GetETTT() << std::endl;
      }
      num_events += m.NumEvents();
      hist_numevts->Fill(m.NumEvents()-.5);
      num_triggers += 1;
    } catch (Exception& e) { e.Dump(); }
  }
  cerr << "total number of triggers: " << num_triggers << endl;
  cerr << "mean number of events per trigger in channel " << channel_id << ": " << ((float)num_events/num_triggers) << endl;
 
  gStyle->SetPadGridX(true); gStyle->SetPadGridY(true);
  gStyle->SetPadTickX(true); gStyle->SetPadTickY(true);
 
  TCanvas* c_time = new TCanvas;
  TLegend* leg = new TLegend(0.15, 0.3, 0.35, 0.4);
  hist_lead->Draw();
  leg->AddEntry(hist_lead, "Leading edge");
  hist_trail->Draw("same");
  hist_trail->SetLineColor(kRed+1);
  leg->AddEntry(hist_trail, "Trailing edge");
  hist_lead->GetXaxis()->SetTitle("Hit edge time (ns)");
  hist_lead->GetYaxis()->SetTitle(Form("Events in channel %d",channel_id));
  leg->Draw();
  c_time->SaveAs("dist_edgetime.png");

  gStyle->SetOptStat(0);
  TCanvas* c_time_zoom = new TCanvas;
  TLegend* leg2 = new TLegend(0.45, 0.75, 0.85, 0.85);
  hist_lead_zoom->Draw();
  hist_trail_zoom->Draw("same");
  hist_trail_zoom->SetLineColor(kRed+1);
  hist_lead_zoom->GetXaxis()->SetTitle("Hit edge time (ns)");
  hist_lead_zoom->GetYaxis()->SetTitle(Form("Events in channel %d",channel_id));
  /*hist_lead_zoom->Fit("gaus", "0");
  hist_trail_zoom->Fit("gaus", "0");
  TF1* f1 = (TF1*)hist_lead_zoom->GetFunction("gaus");
  TF1* f2 = (TF1*)hist_trail_zoom->GetFunction("gaus");
  leg2->AddEntry(hist_lead_zoom, Form("Leading edge  #mu=%.3g, #sigma=%.3g",f1->GetParameter(1),f1->GetParameter(2)), "l");
  leg2->AddEntry(hist_trail_zoom, Form("Trailing edge    #mu=%.3g, #sigma=%.3g",f2->GetParameter(1),f2->GetParameter(2)), "l");
  leg2->Draw();*/
  c_time_zoom->SaveAs("dist_edgetime_zoom.png");
  c_time_zoom->SaveAs("dist_edgetime_zoom.pdf");
  cout << "integral: " << hist_lead_zoom->Integral() << " / " << hist_trail_zoom->Integral() << endl;

  gStyle->SetOptStat(1111);
  TCanvas* c_tot = new TCanvas;
  hist_tot->Draw();
  hist_tot->GetXaxis()->SetTitle("Time over threshold (ns)");
  hist_tot->GetYaxis()->SetTitle(Form("Events in channel %d",channel_id));
  hist_tot->GetYaxis()->SetTitleOffset(1.45);
  c_tot->SaveAs("dist_tot.png");
  c_tot->SaveAs("dist_tot.pdf");
  c_tot->SetLogy();
  c_tot->SaveAs("dist_tot_logscale.png");

  TCanvas* c_nevts = new TCanvas;
  hist_numevts->Draw();
  hist_numevts->GetXaxis()->SetTitle(Form("Hits multiplicity in channel %d / trigger",channel_id));
  hist_numevts->GetYaxis()->SetTitle("Triggers");
  c_nevts->SaveAs("dist_nevts.png");

  return 0;
}
