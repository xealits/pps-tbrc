#include "TCanvas.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TDatime.h"

namespace DQM
{
  /**
   * \author Laurent Forthomme <laurent.forthomme@cern.ch>
   * \date 3 Aug 2015
   */
  class PPSCanvas : public TCanvas
  {
    public:
      inline PPSCanvas() :
        TCanvas("null"), fLegend(0), fLegendX(.52), fLegendY(.76), fLegendNumEntries(0),
        fUpperLabel(0), fLabelsDrawn(false), fRunId(0), fRunDate(TDatime().AsString()) {;}
      inline PPSCanvas(TString name, unsigned int width=500, unsigned int height=500, TString upper_label="") :
        TCanvas(name, "", width, height), fWidth(width), fHeight(height), fLegend(0), fLegendX(.52), fLegendY(.76), fLegendNumEntries(0),
        fUpperLabelText(upper_label), fUpperLabel(0), fLabelsDrawn(false),
        fRunId(0), fRunDate(TDatime().AsString()) { Build(); }
      inline PPSCanvas(TString name, TString upper_label) :
        TCanvas(name, "", 500, 500), fWidth(500), fHeight(500), fLegend(0), fLegendX(.52), fLegendY(.76), fLegendNumEntries(0),
        fUpperLabelText(upper_label), fUpperLabel(0), fLabelsDrawn(false),
        fRunId(0), fRunDate(TDatime().AsString()) { Build(); }
      inline virtual ~PPSCanvas() {
        if (fLegend) delete fLegend;
        if (fUpperLabel) delete fUpperLabel;
      }

      inline void SetRunInfo(unsigned int run_id, TString date) {
        fRunId = run_id;
        fRunDate = date;
      }

      inline void SetUpperLabel(TString text) {
        fUpperLabelText = text;
        fUpperLabel = new TPaveText(.45, .922, .885, .952, "NDC");
        fUpperLabel->SetMargin(0.);
        fUpperLabel->SetFillColor(kWhite);
        fUpperLabel->SetLineColor(kWhite);
        fUpperLabel->SetLineWidth(0);
        fUpperLabel->SetShadowColor(kWhite);
        fUpperLabel->SetTextFont(43);
        fUpperLabel->SetTextAlign(33);
        fUpperLabel->SetTextSize(18);
        fUpperLabel->AddText(fUpperLabelText);
        fUpperLabel->Draw();
      }

      inline TPad* Grid() { return c1; }

      inline void Save(TString ext="png", TString path=".") {
        bool valid_ext = true;
        valid_ext |= (strcmp(ext, "png")!=0);
        valid_ext |= (strcmp(ext, "pdf")!=0);
        if (!valid_ext) return;
        if (!fLabelsDrawn) {
          fLabel1 = new TPaveText(.112, .925, .17, .955, "NDC");
          fLabel1->AddText("PPS");
          fLabel1->SetMargin(0.);
          fLabel1->SetFillColor(kWhite);
          fLabel1->SetLineColor(kWhite);
          fLabel1->SetLineWidth(0);
          fLabel1->SetShadowColor(kWhite);
          fLabel1->SetTextFont(63);
          fLabel1->SetTextAlign(13);
          fLabel1->SetTextSize(22);
          fLabel1->Draw();
          fLabel2 = new TPaveText(.21, .925, .36, .955, "NDC");
          fLabel2->AddText("TB2015");
          fLabel2->SetMargin(0.);
          fLabel2->SetFillColor(kWhite);
          fLabel2->SetLineColor(kWhite);
          fLabel2->SetLineWidth(0);
          fLabel2->SetShadowColor(kWhite);
          fLabel2->SetTextFont(43);
          fLabel2->SetTextAlign(13);
          fLabel2->SetTextSize(22);
          fLabel2->Draw();
          fLabel3 = new TPaveText(.5, .0, .98, .05, "NDC");
          fLabel3->AddText(Form("Run %d - %s", fRunId, fRunDate.Data()));
          fLabel3->SetMargin(0.);
          fLabel3->SetFillColor(kWhite);
          fLabel3->SetLineColor(kWhite);
          fLabel3->SetLineWidth(0);
          fLabel3->SetShadowColor(kWhite);
          fLabel3->SetTextFont(43);
          fLabel3->SetTextAlign(32);
          fLabel3->SetTextSize(16);
          fLabel3->Draw();
          if (fLegend->GetNRows()!=0) fLegend->Draw();
          SetUpperLabel(fUpperLabelText);
          fLabelsDrawn = true;
        }
        TCanvas::SaveAs(Form("%s/%s.%s", path.Data(), TCanvas::GetName(), ext.Data()));
        c1->SetLogy();
        TCanvas::SaveAs(Form("%s/%s_logscale.%s", path.Data(), TCanvas::GetName(), ext.Data()));
      }

    private:
      inline void Build() {
        fLegend = new TLegend(fLegendX, fLegendY, fLegendX+.35, fLegendY+.12);
        fLegend->SetFillColor(kWhite);
        fLegend->SetLineColor(kWhite);
        fLegend->SetLineWidth(0);
        fLegend->SetTextFont(43);
        fLegend->SetTextSize(14);
        DrawGrid();
      }
      inline void DrawGrid() {
        TCanvas::cd();
        gStyle->SetOptStat(0);

        TCanvas::Divide(1,2);
        c1 = (TPad*)TCanvas::GetPad(1);
        c2 = (TPad*)TCanvas::GetPad(2);
        c1->SetPad(0.,0.,1.,1.);
        c2->SetPad(0.,0.,1.,0.);
        c1->SetBottomMargin(0.12);
        c1->SetLeftMargin(0.1);
        c1->SetRightMargin(0.115);
        c1->SetTopMargin(0.1);
        TCanvas::cd(1);
        
        c1->SetTicks(1, 1);
        //c1->SetGrid(1, 1);
      }
      
      TPad *c1, *c2;
      double fWidth, fHeight;
      TLegend *fLegend;
      double fLegendX, fLegendY;
      unsigned int fLegendNumEntries;
      TPaveText *fLabel1, *fLabel2, *fLabel3;
      TString fUpperLabelText;
      TPaveText *fUpperLabel;
      bool fLabelsDrawn;
      unsigned int fRunId;
      TString fRunDate;
  };
}
