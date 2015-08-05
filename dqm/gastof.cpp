#include "FileReader.h"
#include "DQMProcess.h"
#include "GastofCanvas.h"

using namespace std;

bool
GastofDQM(unsigned int address, string filename, vector<string>* outputs)
{
cout << "filename: " << filename << endl;
  FileReader reader;
  try { reader.Open(filename); } catch (Exception& e) { throw e; }
  if (!reader.IsOpen()) throw Exception(__PRETTY_FUNCTION__, "Failed to build FileReader", JustWarning);
  cout << "Run/Burst Id = " << reader.GetRunId() << " / " << reader.GetBurstId() << endl;

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
  canv[kDensity] = new DQM::GastofCanvas(Form("gastof_channels_density_%d_%d_%d", reader.GetRunId(), reader.GetBurstId(), address), "Channels density");
  canv[kMeanToT] = new DQM::GastofCanvas(Form("gastof_mean_tot_%d_%d_%d", reader.GetRunId(), reader.GetBurstId(), address), "Mean ToT (ns)");
  canv[kTriggerTimeDiff] = new DQM::GastofCanvas(Form("gastof_trigger_time_difference_%d_%d_%d", reader.GetRunId(), reader.GetBurstId(), address), "Time btw. each trigger (ns)");

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
        if (trigger_td!=0) { canv[kTriggerTimeDiff]->FillChannel(nino_board, ch_id, (m.GetLeadingTime(0)-trigger_td)*25./1.e3); }
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
      cout << dec;
      cout << "Finished extracting channel " << i << ": " << num_events[i] << " measurements, "
           << "mean number of hits: " << mean_num_events[i] << ", "
           << "mean tot: " << mean_tot[i] << endl;
      reader.Clear();
    } catch (Exception& e) {
      e.Dump();
      if (e.ErrorNumber()<41000) throw e;
    }
  }
  for (unsigned int i=0; i<num_plots; i++) {
    canv[i]->SetRunInfo(address, reader.GetRunId(), reader.GetBurstId(), TDatime().AsString());
    canv[i]->Save("png", DQM_OUTPUT_DIR);
    outputs->push_back(canv[i]->GetName());
  }
  return true;
}

int
main(int argc, char* argv[])
{
  DQM::DQMProcess dqm(1987, 1);
  dqm.Run(GastofDQM);
  return 0;
}
