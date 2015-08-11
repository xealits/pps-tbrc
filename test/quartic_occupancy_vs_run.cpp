#include "FileReader.h"
#include "QuarticCanvas.h"
#include <dirent.h>

using namespace std;

int main(int argc, char* argv[]) {
  if (argc<4) { cerr << "Usage: " << argv[0] << " <board id> <run id> <trigger start> [trigger stop=-1]" << endl; exit(0); }
  const unsigned int num_channels = 32;
  int board_id = atoi(argv[1]);
  int run_id = atoi(argv[2]);
  int trigger_start = atoi(argv[3]), trigger_stop = -1;
  if (argc>4) trigger_stop = atoi(argv[4]);

  DQM::QuarticCanvas* occ = new DQM::QuarticCanvas(Form("occupancy_run%d_triggers%d-%d_board%d", run_id, trigger_start, trigger_stop, board_id), "Integrated occup. / trigger");
  DQM::QuarticCanvas* start = new DQM::QuarticCanvas(Form("leadingedge_run%d_triggers%d-%d_board%d", run_id, trigger_start, trigger_stop, board_id), "Mean lead.edge / trigger");

  ostringstream search1, search2, file;
  search2 << "_board" << board_id << ".dat";
  DIR* dir; struct dirent* ent;
  cout << "Search in directory: " << getenv("PPS_DATA_PATH") << endl;
  VME::TDCMeasurement m; VME::TDCEvent e;
  int num_triggers = 0, num_measurements_per_trigger[num_channels];
  double time_lead[num_channels];
  for (int sp=1; sp<10000000; sp++) { // we loop over all spills
    search1.str(""); search1 << "events_" << run_id << "_" << sp << "_";
    bool file_found = false; string filename;
    // first we search for the proper file to open
    if ((dir=opendir(getenv("PPS_DATA_PATH")))==NULL) return -1;
    while ((ent=readdir(dir))!=NULL) {
      if (string(ent->d_name).find(search1.str())!=string::npos and 
          string(ent->d_name).find(search2.str())!=string::npos) {
        file_found = true;
        filename = ent->d_name;
        break;
      }
    }
    closedir(dir);
    if (!file_found) {
      cout << "Found " << sp << " files in this run" << endl;
      break;
    }
    for (unsigned int i=0; i<num_channels; i++) {
      time_lead[i] = 0.;
      num_measurements_per_trigger[i] = 0;
    }
    // then we open it
    file.str(""); file << getenv("PPS_DATA_PATH") << "/" << filename;
    cout << "Opening file " << file.str() << endl;
    try {
      FileReader f(file.str());
      while (true) {
        if (!f.GetNextEvent(&e)) break;
        if (e.GetType()==VME::TDCEvent::GlobalHeader) {
          for (unsigned int i=0; i<num_channels; i++) {
            time_lead[i] = 0.;
            num_measurements_per_trigger[i] = 0;
          }
          num_triggers++;
        }
        if (num_triggers>trigger_stop and trigger_stop>0) break;
        if (num_triggers>=trigger_start and e.GetType()==VME::TDCEvent::TDCMeasurement and !e.IsTrailing()) {
          occ->FillChannel(e.GetChannelId(), 1);
          unsigned int ch_id = e.GetChannelId();
          time_lead[ch_id] += e.GetTime()*25./1024.;
          num_measurements_per_trigger[ch_id]++;
        }
        if (e.GetType()==VME::TDCEvent::GlobalTrailer) {
          for (unsigned int i=0; i<num_channels; i++) {
            if (num_measurements_per_trigger[i]==0) continue;
            cout << "channel " << i << ": " << time_lead[i]/num_measurements_per_trigger[i] << endl;
            start->FillChannel(i, time_lead[i]/num_measurements_per_trigger[i]);
          }
        }
      }
      //f.Clear(); // we return to beginning of the file
    } catch (Exception& e) { e.Dump(); }
  }
  cout << "num events = " << occ->Grid()->GetSumOfWeights() << endl;
  cout << "num triggers = " << num_triggers << endl;
  occ->Grid()->SetMaximum(0.15);
  occ->Grid()->Scale(1./(num_triggers-trigger_start));
  occ->SetRunInfo(board_id, run_id, 0, Form("Triggers %d-%d", trigger_start, trigger_stop));
  occ->Save(Form("png"));

  occ->Grid()->SetMaximum(1.e5);
  start->Grid()->Scale(1./(num_triggers-trigger_start));
  start->SetRunInfo(board_id, run_id, 0, Form("Triggers %d-%d", trigger_start, trigger_stop));
  start->Save(Form("png"));

  return 0;
}
