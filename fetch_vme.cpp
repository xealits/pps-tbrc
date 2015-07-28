#include "VMEReader.h"
#include "FileConstants.h"

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <ctime>
#include <signal.h>

using namespace std;

VMEReader* vme;
int gEnd = 0;

void CtrlC(int aSig) {
  if (gEnd==0) { cerr << endl << "[C-c] Trying a clean exit!" << endl; vme->Abort(); }
  else if (gEnd>=5) { cerr << endl << "[C-c > 5 times] ... Forcing exit!" << endl; exit(0); }
  gEnd++;
}

int main(int argc, char *argv[]) {
  signal(SIGINT, CtrlC);
  
  const unsigned int num_tdc = 1;

  fstream out_file[num_tdc];
  unsigned int num_events[num_tdc];

  VME::TDCEventCollection ec;
  VME::TDCV1x90* tdc[num_tdc];
  VME::FPGAUnitV1495* fpga;

  VME::AcquisitionMode acq_mode = VME::TRIG_MATCH;
  VME::DetectionMode det_mode = VME::TRAILEAD;
  
  file_header_t fh;
  fh.magic = 0x30535050; // PPS0 in ASCII
  fh.run_id = 0;
  fh.spill_id = 0;
  fh.acq_mode = acq_mode;
  fh.det_mode = det_mode;
  
  std::time_t t_beg;
  for (unsigned int i=0; i<num_tdc; i++) num_events[i] = 0;
  unsigned int num_triggers = 0;
  bool use_fpga = true;

  try {
    bool with_socket = false;
    vme = new VMEReader("/dev/a2818_0", VME::CAEN_V2718, with_socket);
    vme->ReadXML(argv[1]);
  
    static const unsigned int num_tdc = vme->GetNumTDC();
    
    VME::FPGAUnitV1495* fpga = vme->GetFPGAUnit();
    const bool use_fpga = (fpga!=0);
    fstream out_file[num_tdc];
    string acqmode[num_tdc], detmode[num_tdc];
    unsigned int num_events[num_tdc];

    VME::TDCCollection tdcs = vme->GetTDCCollection(); unsigned int i=0;
    for (VME::TDCCollection::iterator atdc=tdcs.begin(); atdc!=tdcs.end(); atdc++, i++) {
      VME::TDCV1x90* tdc = atdc->second;
      
      file_header_t fh;
      fh.magic = 0x30535050; // PPS0 in ASCII
      fh.run_id = 0;
      fh.spill_id = 0;
      fh.acq_mode = tdc->GetAcquisitionMode();
      fh.det_mode = tdc->GetDetectionMode();
  
      std::ostringstream filename;
      filename << "events_board" << i << ".dat";
      vme->SetOutputFile(filename.str());
      //filename << GenerateFileName(0);
      out_file[i].open(vme->GetOutputFile().c_str(), fstream::out | ios::binary );	
      if (!out_file[i].is_open()) {
        throw Exception(__PRETTY_FUNCTION__, "Error opening file", Fatal);
      }
      out_file[i].write((char*)&fh, sizeof(file_header_t));

      switch (fh.acq_mode) {
        case VME::CONT_STORAGE: acqmode[i] = "Continuous storage"; break;
        case VME::TRIG_MATCH: acqmode[i] = "Trigger matching"; break;
        default:
          acqmode[i] = "[Invalid mode]";
          throw Exception(__PRETTY_FUNCTION__, "Invalid acquisition mode!", Fatal);
      }
      switch (fh.det_mode) {
        case VME::PAIR: detmode[i] = "Pair measurement"; break;
        case VME::OLEADING: detmode[i] = "Leading edge only"; break;
        case VME::OTRAILING: detmode[i] = "Trailing edge only"; break;
        case VME::TRAILEAD: detmode[i] = "Leading and trailing edges"; break;
      }
    } 

    if (use_fpga) {
      fpga->PulseTDCBits(VME::FPGAUnitV1495::kReset|VME::FPGAUnitV1495::kClear); // send a RST+CLR signal from FPGA to TDCs
      fpga->StartScaler();
    }

    t_beg = std::time(0);

    cerr << endl << "*** Ready for acquisition! ***" << endl
         << "Acquisition mode: ";
    for (unsigned int i=0; i<num_tdc; i++) { if (i>0) cerr << " / "; cerr << acqmode[i]; }
    cerr << endl 
         << "Detection mode: ";
    for (unsigned int i=0; i<num_tdc; i++) { if (i>0) cerr << " / "; cerr << detmode[i]; }
    cerr << endl 
         << "Local time: " << asctime(std::localtime(&t_beg));

    for (unsigned int i=0; i<num_tdc; i++) num_events[i] = 0;
    while (true) {
      for (VME::TDCCollection::iterator atdc=tdcs.begin(); atdc!=tdcs.end(); atdc++) {
        ec = atdc->second->FetchEvents();
        if (ec.size()==0) continue; // no events were fetched
        for (VME::TDCEventCollection::const_iterator e=ec.begin(); e!=ec.end(); e++) {
          uint32_t word = e->GetWord();
          out_file[i].write((char*)&word, sizeof(uint32_t));
          /*cout << hex << e->GetType() << endl;
          if (e->GetType()==VME::TDCEvent::TDCMeasurement) cout << e->GetChannelId() << endl;*/
        }
        num_events[i] += ec.size();
      }
      if (use_fpga) {
        num_triggers = fpga->GetScalerValue(); // FIXME need to probe this a bit less frequently
        if (num_triggers>0 and num_triggers%1000==0) cerr << "--> " << num_triggers << " triggers acquired in this run so far" << endl;
      }
    }
  } catch (Exception& e) {
    if (e.ErrorNumber()==TDC_ACQ_STOP) {
      for (unsigned int i=0; i<num_tdc; i++) {
        if (out_file[i].is_open()) out_file[i].close();
      }

      std::time_t t_end = std::time(0);
      double nsec_tot = difftime(t_end, t_beg), nsec = fmod(nsec_tot,60), nmin = (nsec_tot-nsec)/60.;
      cerr << endl << "*** Acquisition stopped! ***" << endl
           << "Local time: " << asctime(std::localtime(&t_end))
           << "Total acquisition time: " << difftime(t_end, t_beg) << " seconds"
           << " (" << nmin << " min " << nsec << " sec)"
           << endl;
    
      cerr << endl << "Acquired ";
      for (unsigned int i=0; i<num_tdc; i++) { if (i>0) cerr << " / "; cerr << num_events[i]; }
      cerr << " words for " << num_triggers << " triggers in this run" << endl;
      //if (use_fpga) fpga->StopScaler();
  
      delete vme;
      return 0;
    }
    e.Dump();
    return -1;
  }
    
  return 0;
}
