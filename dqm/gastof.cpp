#include "FileReader.h"
#include "Client.h"
#include "GastofCanvas.h"
#include "Exception.h"
#include <iostream>

#define DQM_OUTPUT_DIR "/tmp/"

using namespace std;

bool
RunGastofDQM(unsigned int address, string filename, vector<string>* outputs)
{
  FileReader reader;
  try { reader.Open(filename); } catch (Exception& e) { throw e; }
  if (!reader.IsOpen()) throw Exception(__PRETTY_FUNCTION__, "Failed to build FileReader", JustWarning);

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
  canv[kDensity] = new DQM::GastofCanvas(Form("gastof_channels_density_%x", address), "Channels density");
  canv[kMeanToT] = new DQM::GastofCanvas(Form("gastof_mean_tot_%x", address), "Mean ToT (ns)");
  canv[kTriggerTimeDiff] = new DQM::GastofCanvas(Form("gastof_trigger_time_difference_%x", address), "Time btw. each trigger (ns)");

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
      if (e.ErrorNumber()<41000) throw e;
    }
  }
  for (unsigned int i=0; i<num_plots; i++) {
    canv[i]->SetRunInfo(address, reader.GetRunId(), reader.GetSpillId(), "now()");
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
      if (msg.GetKey()!=NEW_FILENAME) { continue;
        //ostringstream os; os << "Invalid message key retrieved: " << MessageKeyToString(msg.GetKey());
        //throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
      }
      if (msg.GetValue()=="") {
        ostringstream os; os << "Invalid output file path received through the NEW_FILENAME message: " << msg.GetValue();
        throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
      }
      vector<string> outputs; string value = msg.GetValue();
      
      size_t end; uint32_t board_address; string filename;
      if ((end=value.find(':'))==std::string::npos) {
        ostringstream s; s << "Invalid filename message built! (\"" << value << "\")";
        throw Exception(__PRETTY_FUNCTION__, s.str().c_str(), JustWarning);
      }
      board_address = atoi(value.substr(0, end).c_str());
      filename = value.substr(end+1);
      //cout << "Board address 0x" << hex << board_address << " has output file " << filename << endl;

      if (RunGastofDQM(board_address, filename, &outputs)) {
        cout << "Produced " << outputs.size() << " plot(s) for board with address 0x" << hex << board_address << endl;
        for (vector<string>::iterator nm=outputs.begin(); nm!=outputs.end(); nm++) {
          client.Send(SocketMessage(NEW_DQM_PLOT, *nm)); usleep(1000);
        }
      }
    }
  } catch (Exception& e) {
    e.Dump();
    client.Send(e);
  }
  return 0;
}
