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

  enum plots {
    kNumWords,
    numPlots
  }
  const num_plots = numPlots;
  DQM::PPSCanvas* canv[num_plots];

  canv[kNumWords] = new DQM::PPSCanvas("multiread_num_words_per_file", "Number of data words per file");

  vector<string> files;
  string buff;
  fstream list(argv[1]);
  while (list>>buff) { files.push_back(buff); }

  TH1D* h_num_words = new TH1D("num_words", "", 300, 0., 300000.);

  for (vector<string>::iterator f=files.begin(); f!=files.end(); f++) {
    try {
      FileReader fr(*f);
      h_num_words->Fill(fr.GetNumEvents());
      
    } catch (Exception& e) {
      e.Dump();
    }
  }

  //canv[kNumWords]->cd();
  h_num_words->Draw();
  //canv[kNumWords]->Grid()->SetLogx();
  canv[kNumWords]->Save("png");

  return 0;
}
