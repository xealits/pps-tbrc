#include "FileReader.h"
#include "Client.h"
#include "GastofCanvas.h"
#include <iostream>

using namespace std;

int
main(int argc, char* argv[])
{
  if (argc<2) {
    cerr << "Usage: " << argv[0] << " <input filename>" << endl;
    return -1;
  }

  FileReader reader;
  try {
    reader.Open(argv[1]);
    if (!reader.IsOpen()) return -1;
  } catch (Exception& e) { e.Dump(); }

  Client client(1987);

  const unsigned int num_channels = 64;
  double mean_num_events[num_channels], mean_tot[num_channels];
  unsigned int num_events[num_channels];

  enum plots {
    kDensity,
    kMeanToT,
    kNumPlots
  };
  const unsigned short num_plots = kNumPlots;
  DQM::GastofCanvas* canv[num_plots];
  canv[kDensity] = new DQM::GastofCanvas("gastof_channels_density", "Channels density");
  canv[kMeanToT] = new DQM::GastofCanvas("gastof_mean_tot", "Mean ToT (ns)");

  VME::TDCMeasurement m;
  for (unsigned int i=0; i<num_channels; i++) {
    mean_num_events[i] = mean_tot[i] = 0.;
    num_events[i] = 0;
    try {
      while (true) {
        if (!reader.GetNextMeasurement(i, &m)) break;
        for (unsigned int j=0; j<m.NumEvents(); j++) {
          mean_tot[j] += m.GetToT(j)*25./1.e3/m.NumEvents();
        }
        mean_num_events[i] += m.NumEvents();
        if (m.NumEvents()!=0) num_events[i] += 1;
      }
      if (num_events[i]>0) {
        mean_num_events[i] /= num_events[i];
        mean_tot[i] /= num_events[i];
      }
      canv[kDensity]->FillChannel(i, mean_num_events[i]);
      canv[kMeanToT]->FillChannel(i, mean_tot[i]);
      cout << "Finished extracting channel " << i << ": " << num_events[i] << " measurements, mean number of hits: " << mean_num_events[i] << endl;
      reader.Clear();
    } catch (Exception& e) { e.Dump(); }
  }
  for (unsigned int i=0; i<num_plots; i++) {
    canv[i]->DrawGrid(); canv[i]->Save();
  }

  return 0;
}
