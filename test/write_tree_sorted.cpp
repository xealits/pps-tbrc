#include "FileReader.h"
#include "QuarticCanvas.h"
#include <dirent.h>

#include "TFile.h"
#include "TTree.h"

#define MAX_MEAS 10000

using namespace std;

int main(int argc, char* argv[]) {
  if (argc<4) { cerr << "Usage: " << argv[0] << " <board id> <run id> <trigger start> [trigger stop=-1]" << endl; exit(0); }
  const unsigned int num_channels = 32;
  int board_id = atoi(argv[1]);
  int run_id = atoi(argv[2]);
  int trigger_start = atoi(argv[3]), trigger_stop = -1;
  if (argc>4) trigger_stop = atoi(argv[4]);
  
  string output = "output_test_sorted.root";
  if (argc>5) output = argv[5];

  int fNumMeasurements, fNumErrors;
  int fRunId, fBurstId;
  int fETTT, fTriggerNumber;
  int fChannelId[MAX_MEAS];
  double fLeadingEdge[MAX_MEAS], fTrailingEdge[MAX_MEAS], fToT[MAX_MEAS];

  TFile* f = new TFile(output.c_str(), "recreate");
  TTree* t = new TTree("tdc", "List of TDC measurements");
  t->Branch("num_measurements", &fNumMeasurements, "num_measurements/I");
  t->Branch("num_errors", &fNumErrors, "num_errors/I");
  t->Branch("run_id", &fRunId, "run_id/I");
  //t->Branch("burst_id", &fBurstId, "burst_id/I"); // need to be clever for this...
  t->Branch("channel_id", fChannelId, "channel_id[num_measurements]/I");
  t->Branch("leading_edge", fLeadingEdge, "leading_edge[num_measurements]/D");
  t->Branch("trailing_edge", fTrailingEdge, "trailing_edge[num_measurements]/D");
  t->Branch("tot", fToT, "tot[num_measurements]/D");
  t->Branch("ettt", &fETTT, "ettt/I");
  t->Branch("trigger_number",&fTriggerNumber,"trigger_number/I");
  
  ostringstream search1, search2, file;
  search2 << "_board" << board_id << ".dat";
  DIR* dir; struct dirent* ent;
  cout << "Search in directory: " << getenv("PPS_DATA_PATH") << endl;
  VME::TDCMeasurement m; VME::TDCEvent e;
  int num_triggers = 0, num_channel_measurements[num_channels];
  double has_leading_per_trigger[num_channels];

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
      num_channel_measurements[i] = 0;
      has_leading_per_trigger[i] = 0;
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
            num_channel_measurements[i] = 0;
	    has_leading_per_trigger[i] = 0;
          }
          num_triggers++;
	  if(num_triggers % 1000 == 0)
	    cout << "Triggers received: " << num_triggers << endl;
	  fNumMeasurements = fNumErrors = 0;
          if (num_triggers>trigger_stop and trigger_stop>0) break;
          else if (num_triggers<trigger_start) continue;
	  //	  cout << "GlobalHeader, trigger number " << fTriggerNumber << endl;
	  fTriggerNumber = num_triggers;
        }

        else if (e.GetType()==VME::TDCEvent::TDCMeasurement) {
          unsigned int ch_id = e.GetChannelId();
          if (!e.IsTrailing()) {
            has_leading_per_trigger[ch_id] =  e.GetTime()*25./1024.;
	    //	  cout << "\tTDCMeasurement Leading edge: channel " << ch_id << ": " << has_leading_per_trigger[ch_id] << endl;
          }
          else {
	    double trailing_time = e.GetTime()*25./1024.;
	    //	  cout << "\tTDCMeasurement Trailing edge: channel " << ch_id << ": " << trailing_time << endl;
	    //	  cout << "\tTDCMeasurement Trailing edge: channel " << ch_id << ": " << trailing_time-has_leading_per_trigger[ch_id] << endl;
            if (has_leading_per_trigger[ch_id]==0.) continue;

	    fChannelId[fNumMeasurements] = ch_id;
	    fLeadingEdge[fNumMeasurements] = has_leading_per_trigger[ch_id];
	    fTrailingEdge[fNumMeasurements] = trailing_time;
	    fToT[fNumMeasurements] = fTrailingEdge[fNumMeasurements]-fLeadingEdge[fNumMeasurements];
            num_channel_measurements[ch_id]++;
            fNumMeasurements++;
            has_leading_per_trigger[ch_id] = 0.;
          }
        }
        else if (e.GetType()==VME::TDCEvent::ETTT) {
          fETTT = e.GetETTT();
        }
	
        else if (e.GetType()==VME::TDCEvent::GlobalTrailer) {
          //        cout << "GlobalTrailer, trigger number " << fTriggerNumber << endl;
          for (unsigned int i=0; i<num_channels; i++) {
            if (num_channel_measurements[i]==0) continue;
	    //	    cout << "GlobalTrailer: channel num. measurements " << i << ": " << num_channel_measurements[i] << endl;
          }
          if (num_triggers>trigger_stop and trigger_stop>0) break;
          else if (num_triggers<trigger_start) continue;
	  t->Fill();
        }
      }
      //      f.Clear(); // we return to beginning of the file
    } catch (Exception& e) { e.Dump(); }
  }
  t->Write();
  
  return 0;
}
