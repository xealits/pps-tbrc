#include "FileReader.h"
#include "PPSCanvas.h"
#include "QuarticCanvas.h"

#include <iostream>
#include <fstream>
#include <vector>

#include "TH1.h"

using namespace std;

int main(int argc, char* argv[]) {
  if (argc<2) {
    cerr << "Usage: " << argv[0] << " <files list>" << endl;
    return -1;
  }

  enum general_plots {
    kNumWords,
    numGenPlots
  };
  enum quartic_plots {
    kLeadingTime,
    kNumEvents,
    numQuPlots
  };

  const unsigned int num_gen_plots = numGenPlots;
  DQM::PPSCanvas* cgen[num_gen_plots];

  const unsigned int num_qu_plots = numQuPlots;
  DQM::QuarticCanvas* cqu[num_qu_plots];

  vector<string> files;
  string buff;
  fstream list(argv[1]);
  while (list>>buff) { files.push_back(buff); }

  TH1D* h_num_words = new TH1D("num_words", "", 300, 0., 300000.);

  cqu[kLeadingTime] = new DQM::QuarticCanvas("multiread_quartic_mean_leading_time", "Mean leading time (ns)");
  //cqu[kNumEvents] = new DQM::QuarticCanvas("multiread_quartic_num_events_per_channel", "Number of events per channel");

  VME::TDCMeasurement m;
  for (vector<string>::iterator f=files.begin(); f!=files.end(); f++) {
    try {
      FileReader fr(*f);
      cout << "Opening file with burst train " << fr.GetBurstId() << endl;
      h_num_words->Fill(fr.GetNumEvents());
      for (unsigned int ch=0; ch<32; ch++) {
        while (true) {
          if (!fr.GetNextMeasurement(ch, &m)) break;
          //cqu[kNumEvents]->FillChannel(ch, m.NumEvents());
          unsigned int leadingtime = 0;
          for (unsigned int i=0; i<m.NumEvents(); i++) { leadingtime += m.GetLeadingTime(i); }
          cqu[kLeadingTime]->FillChannel(ch, leadingtime*25./1000./m.NumEvents());
        }
        fr.Clear();
      }
    } catch (Exception& e) {
      e.Dump();
    }
  }

  cgen[kNumWords] = new DQM::PPSCanvas("multiread_num_words_per_file", "Number of data words per file");
  //cgen[kNumWords]->cd();
  h_num_words->Draw();
  //cgen[kNumWords]->Grid()->SetLogx();
  cgen[kNumWords]->Save("png");

  //cqu[kNumEvents]->Save("png");
  cqu[kLeadingTime]->Save("png");

  return 0;
}
