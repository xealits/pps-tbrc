#include "FileReader.h"
#include "Client.h"
#include "GastofCanvas.h"
#include "Exception.h"
#include <iostream>

#define DQM_OUTPUT_DIR "/tmp/"

using namespace std;

bool
RunGastofDQM(string filename, vector<string>* outputs)
{
  FileReader reader;
  try {
    reader.Open(filename);
  } catch (Exception& e) { throw e; }
  if (!reader.IsOpen()) return false;

  const unsigned int num_channels = 32;
  double mean_num_events[num_channels], mean_tot[num_channels];
  int trigger_td;
  unsigned int num_events[num_channels];

  enum plots {
    kDensity,
    kMeanToT,
    kTriggerTimeDiff,
    kNumPlots
  };
  const unsigned short num_plots = kNumPlots;
  DQM::GastofCanvas* canv[num_plots];
  canv[kDensity] = new DQM::GastofCanvas("gastof_channels_density", "Channels density");
  canv[kMeanToT] = new DQM::GastofCanvas("gastof_mean_tot", "Mean ToT (ns)");
  canv[kTriggerTimeDiff] = new DQM::GastofCanvas("gastof_trigger_time_difference", "Time btw. each trigger (ns)");

  VME::TDCMeasurement m;
  for (unsigned int i=0; i<num_channels; i++) {
    unsigned short nino_board, ch_id;
    mean_num_events[i] = mean_tot[i] = 0.;
    num_events[i] = 0;
    trigger_td = 0;
    try {
      if (i<32) { nino_board = 1; ch_id = i; }
      else      { nino_board = 0; ch_id = i-32; }
      while (true) {
        if (!reader.GetNextMeasurement(i, &m)) break;
        if (trigger_td!=0.) { canv[kTriggerTimeDiff]->FillChannel(nino_board, ch_id, (m.GetLeadingTime(0)-trigger_td)*25./1.e3); }
        trigger_td = m.GetLeadingTime(0);
        for (unsigned int j=0; j<m.NumEvents(); j++) {
          mean_tot[i] += m.GetToT(j)*25./1.e3/m.NumEvents();
        }
        mean_num_events[i] += m.NumEvents();
        if (m.NumEvents()!=0) num_events[i] += 1;
      }
      if (num_events[i]>0) {
        mean_num_events[i] /= num_events[i];
        mean_tot[i] /= num_events[i];
      }
      canv[kDensity]->FillChannel(nino_board, ch_id, mean_num_events[i]);
      canv[kMeanToT]->FillChannel(nino_board, ch_id, mean_tot[i]);
      cout << "Finished extracting channel " << i << ": " << num_events[i] << " measurements, "
           << "mean number of hits: " << mean_num_events[i] << ", "
           << "mean tot: " << mean_tot[i] << endl;
      reader.Clear();
    } catch (Exception& e) {
      if (e.ErrorNumber()<41000) throw e;
    }
  }
  for (unsigned int i=0; i<num_plots; i++) {
    //canv[i]->SetRunInfo(0, "now()");
    canv[i]->Save("png", DQM_OUTPUT_DIR);
    outputs->push_back(canv[i]->GetName());
  }
  return true;
}

int
main(int argc, char* argv[])
{
  Client client(1987);
  try {
    client.Connect(Socket::DQM);
  } catch (Exception& e) { e.Dump(); }
  try {
    SocketMessage msg;
    while (true) {
      msg = client.Receive(NEW_FILENAME);
      if (msg.GetKey()==INVALID_KEY) continue;
      vector<string> outputs;
      if (RunGastofDQM(msg.GetCleanedValue(), &outputs)) {
        cout << "Produced " << outputs.size() << " plot(s)" << endl;
        for (vector<string>::iterator nm=outputs.begin(); nm!=outputs.end(); nm++) {
          client.Send(SocketMessage(NEW_DQM_PLOT, *nm));
        }
      }
    }
  } catch (Exception& e) {
    e.Dump();
    client.Send(e);
  }
  return 0;
}
