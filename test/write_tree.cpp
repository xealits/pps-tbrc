#include "FileReader.h"

#include <iostream>
#include <fstream>
#include <vector>

#include "TFile.h"
#include "TTree.h"

#define MAX_MEAS 10000

using namespace std;

int main(int argc, char* argv[]) {
  if (argc<2) {
    cerr << "Usage: " << argv[0] << " <input raw file> [output root file]" << endl;
    return -1;
  }

  string output = "output.root";
  if (argc>2) output = argv[2];

  unsigned int fNumMeasurements, fNumErrors;
  unsigned int fRunId, fBurstId;
  unsigned long fETTT;
  unsigned int fChannelId[MAX_MEAS];
  double fLeadingEdge[MAX_MEAS], fTrailingEdge[MAX_MEAS], fToT[MAX_MEAS];

  TFile* f = new TFile(output.c_str(), "recreate");
  TTree* t = new TTree("tdc", "List of TDC measurements");
  t->Branch("num_measurements", &fNumMeasurements, "num_measurements/i");
  t->Branch("num_errors", &fNumErrors, "num_errors/i");
  t->Branch("run_id", &fRunId, "run_id/i");
  t->Branch("channel_id", fChannelId, "channel_id[num_measurements]/I");
  //t->Branch("burst_id", &fBurstId, "burst_id/i"); // need to be clever for this...
  t->Branch("ettt", &fETTT, "ettt/l");
  t->Branch("leading_edge", fLeadingEdge, "leading_edge[num_measurements]/D");
  t->Branch("trailing_edge", fTrailingEdge, "trailing_edge[num_measurements]/D");
  t->Branch("tot", fToT, "tot[num_measurements]/D");

  fNumMeasurements = fNumErrors = fRunId = fBurstId = 0;
  for (unsigned int i=0; i<MAX_MEAS; i++) {
    fLeadingEdge[i] = fTrailingEdge[i] = fToT[i] = 0.;
    fChannelId[i] = -1;
  }

  VME::TDCMeasurement m;
  try {
    FileReader fr(argv[1]);
    fRunId = fr.GetRunId();
    cout << "Opening file with burst train " << fr.GetBurstId() << endl;
    for (unsigned int ch=0; ch<32; ch++) {
      while (true) {
        if (!fr.GetNextMeasurement(ch, &m)) break;
        fNumMeasurements = m.NumEvents();
        fNumErrors = m.NumErrors();
        fETTT = m.GetETTT();
        for (unsigned int i=0; i<m.NumEvents(); i++) {
          fLeadingEdge[i] = m.GetLeadingTime(i)*25./1000.;
          fChannelId[i] = ch;
          fTrailingEdge[i] = m.GetTrailingTime(i)*25./1000.;
          fToT[i] = m.GetToT(i)*25./1000.;
        }
        t->Fill();
      }
      fr.Clear();
    }
  } catch (Exception& e) {
    e.Dump();
  }

  f->Write();
  return 0;
}
