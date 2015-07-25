#include "TCanvas.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TH2.h"
#include "TStyle.h"

namespace DQM
{
  /**
   * \author Laurent Forthomme <laurent.forthomme@cern.ch>
   * \date 25 Jul 2015
   */
  class GastofCanvas : public TCanvas
  {
    public:
      inline GastofCanvas() :
        TCanvas("null"), fLegend(0), fLegendX(.52), fLegendY(.76), fLegendNumEntries(0), fUpperLabel(0), fLabelsDrawn(false) {;}
      inline GastofCanvas(TString name, unsigned int width=500, unsigned int height=500, TString upper_label="") :
        TCanvas(name, "", width, height), fWidth(width), fHeight(height), fLegend(0), fLegendX(.52), fLegendY(.76), fLegendNumEntries(0),
        fUpperLabelText(upper_label), fUpperLabel(0), fLabelsDrawn(false) { Build(); }
      inline GastofCanvas(TString name, TString upper_label) :
        TCanvas(name, "", 500, 500), fWidth(500), fHeight(500), fLegend(0), fLegendX(.52), fLegendY(.76), fLegendNumEntries(0),
        fUpperLabelText(upper_label), fUpperLabel(0), fLabelsDrawn(false) { Build(); }
      inline virtual ~GastofCanvas() {
        if (fLegend) delete fLegend;
        if (fUpperLabel) delete fUpperLabel;
        if (fHist) delete fHist;
      }

      void SetUpperLabel(TString text) {
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

      inline void FillChannel(unsigned short channel_id, double content) {
        const Coord c = GetCoordinates(channel_id);
        fHist->Fill(c.x, c.y, content);
      }
      inline void DrawGrid() const { fHist->Draw("colz"); }
      inline TH2D* Grid() { return fHist; }

      inline void Save(TString ext="png") {
        if (strcmp(ext, "png")!=0) return;
        if (!fLabelsDrawn) {
          fLabel1 = new TPaveText(.112, .925, .2, .955, "NDC");
          fLabel1->AddText("GasToF");
          fLabel1->SetMargin(0.);
          fLabel1->SetFillColor(kWhite);
          fLabel1->SetLineColor(kWhite);
          fLabel1->SetLineWidth(0);
          fLabel1->SetShadowColor(kWhite);
          fLabel1->SetTextFont(63);
          fLabel1->SetTextAlign(13);
          fLabel1->SetTextSize(22);
          fLabel1->Draw();
          fLabel2 = new TPaveText(.29, .925, .36, .955, "NDC");
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
          if (fLegend->GetNRows()!=0) fLegend->Draw();
          SetUpperLabel(fUpperLabelText);
          fLabelsDrawn = true;
        }
        TCanvas::SaveAs(Form("%s.%s", TCanvas::GetName(), ext.Data()));
      }

    private:
      inline void Build() {
        gStyle->SetOptStat(0);
        gStyle->SetMarkerStyle(20);
        gStyle->SetMarkerSize(.87);
        gStyle->SetTitle("");
        gStyle->SetTitleFont(43, "XYZ");
        gStyle->SetTitleSize(22, "XYZ");
        //gStyle->SetTitleOffset(2., "Y");
        gStyle->SetLabelFont(43, "XYZ");
        gStyle->SetLabelSize(22, "XY");
        gStyle->SetLabelSize(18, "Z");
        gStyle->SetTitleOffset(1.3, "Y");
            
        TCanvas::Divide(1,2);
        c1 = (TPad*)TCanvas::GetPad(1);
        c2 = (TPad*)TCanvas::GetPad(2);
        c1->SetPad(0.,0.,1.,1.);
        c2->SetPad(0.,0.,1.,0.);
        c1->SetBottomMargin(0.1);
        c1->SetLeftMargin(0.1);
        c1->SetRightMargin(0.115);
        c1->SetTopMargin(0.1);
        TCanvas::cd(1);
        
        fLegend = new TLegend(fLegendX, fLegendY, fLegendX+.35, fLegendY+.12);
        fLegend->SetFillColor(kWhite);
        fLegend->SetLineColor(kWhite);
        fLegend->SetLineWidth(0);
        fLegend->SetTextFont(43);
        fLegend->SetTextSize(14);
        
        c1->SetTicks(1, 1);

        fHist = new TH2D(Form("hist_%s", TCanvas::GetName()), "", 8, 0., 8., 8, 0., 8.);
      }
      
      struct Coord { unsigned int x; unsigned int y; };
      inline Coord GetCoordinates(unsigned short channel_id) const {
        Coord out;
        out.x = out.y = 0;
        return out;
      }

      TPad *c1, *c2;
      TH2D* fHist;
      double fWidth, fHeight;
      TLegend *fLegend;
      double fLegendX, fLegendY;
      unsigned int fLegendNumEntries;
      TPaveText *fLabel1, *fLabel2;
      TString fUpperLabelText;
      TPaveText *fUpperLabel;
      bool fLabelsDrawn;
  };
}
