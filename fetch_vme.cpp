#include "VMEReader.h"
#include "FileConstants.h"
#include "OnlineDBHandler.h"

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <ctime>
#include <signal.h>

#define NUM_TRIG_BEFORE_FILE_CHANGE 10000
#define PATH "."

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
  
  // Where to put the logs
  ofstream err_log("log.err", ios::binary);
  //const Logger lr(err_log, cerr);
 
  string xml_config;
  if (argc<2) {
    cout << "No configuration file provided! using default config/config.xml" << endl;
    xml_config = "config/config.xml";
  }
  else xml_config = argv[1];

  const unsigned int num_tdc = 1;

  ofstream out_file[num_tdc];
  unsigned int num_events[num_tdc];  

  VME::TDCEventCollection ec;

  VME::AcquisitionMode acq_mode = VME::TRIG_MATCH;
  VME::DetectionMode det_mode = VME::TRAILEAD;
  
  file_header_t fh;
  fh.magic = 0x30535050; // PPS0 in ASCII
  fh.spill_id = 0;
  fh.acq_mode = acq_mode;
  fh.det_mode = det_mode;
  
  time_t t_beg;
  for (unsigned int i=0; i<num_tdc; i++) num_events[i] = 0;
  unsigned long num_triggers = 0, num_all_triggers = 0, num_files = 0;

  try {
    bool with_socket = true;

    OnlineDBHandler("run_infos.db").NewRun();
    // Initialize the configuration one single time
    vme = new VMEReader("/dev/a2818_0", VME::CAEN_V2718, with_socket);
    try { vme->ReadXML(xml_config); } catch (Exception& e) {
      if (vme->UseSocket()) vme->Send(e);
    }
 
    fh.run_id = vme->GetRunNumber();
  
    static const unsigned int num_tdc = vme->GetNumTDC();
    
    VME::FPGAUnitV1495* fpga = vme->GetFPGAUnit();
    const bool use_fpga = (fpga!=0);
    fstream out_file[num_tdc];
    string acqmode[num_tdc], detmode[num_tdc];
    unsigned int num_events[num_tdc];
    int num_triggers_in_files;

    t_beg = time(0);

    // Initial dump of the acquisition parameters before writing the files
    cerr << endl << "*** Ready for acquisition! ***" << endl;
    VME::TDCCollection tdcs = vme->GetTDCCollection(); unsigned int i = 0;
    for (VME::TDCCollection::iterator atdc=tdcs.begin(); atdc!=tdcs.end(); atdc++, i++) {
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
    cerr << "Acquisition mode: ";
    for (unsigned int i=0; i<num_tdc; i++) { if (i>0) cerr << " / "; cerr << acqmode[i]; }
    cerr << endl 
         << "Detection mode: ";
    for (unsigned int i=0; i<num_tdc; i++) { if (i>0) cerr << " / "; cerr << detmode[i]; }
    cerr << endl 
         << "Local time: " << asctime(localtime(&t_beg));
      

    // Change outputs file once a minimal amount of triggers is hit
    do {
      // TDC output files configuration
      i = 0;
      fh.spill_id += 1;
      time_t start = time(0);
      OnlineDBHandler("run_infos.db").NewBurst();
      for (VME::TDCCollection::iterator atdc=tdcs.begin(); atdc!=tdcs.end(); atdc++, i++) {
        VME::TDCV1x90* tdc = atdc->second;
        
        fh.acq_mode = tdc->GetAcquisitionMode();
        fh.det_mode = tdc->GetDetectionMode();

        ostringstream filename;
        filename << PATH << "/events"
                 << "_" << fh.run_id
                 << "_" << fh.spill_id
                 << "_" << start
                 << "_board" << i
                 //<< "_" GenerateString(4)
                 << ".dat";
        vme->SetOutputFile(atdc->first, filename.str());
        out_file[i].open(vme->GetOutputFile(atdc->first).c_str(), fstream::out | ios::binary);
        if (!out_file[i].is_open()) {
          throw Exception(__PRETTY_FUNCTION__, "Error opening file", Fatal);
        }
        out_file[i].write((char*)&fh, sizeof(file_header_t));
        
      }
      
      // Pulse to set a common starting time for both TDC boards
      if (use_fpga) {
        fpga->PulseTDCBits(VME::FPGAUnitV1495::kReset|VME::FPGAUnitV1495::kClear); // send a RST+CLR signal from FPGA to TDCs
        fpga->StartScaler();
      }
      
      num_triggers_in_files = 0;

      // Data readout from the two TDC boards
      for (unsigned int i=0; i<num_tdc; i++) { num_events[i] = 0; }
      unsigned long tm = 0;
      while (true) {
        unsigned int i = 0; tm += 1;
        for (VME::TDCCollection::iterator atdc=tdcs.begin(); atdc!=tdcs.end(); atdc++, i++) {
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
        if (use_fpga and tm>100000) {
          num_triggers = fpga->GetScalerValue(); // FIXME need to probe this a bit less frequently
          num_triggers_in_files = num_triggers-num_all_triggers;
          if (num_triggers>0 and num_triggers%1000==0) cerr << "--> " << num_triggers << " triggers acquired in this run so far" << endl;
          if (num_triggers_in_files>0 and num_triggers_in_files>=NUM_TRIG_BEFORE_FILE_CHANGE) {
            num_all_triggers = num_triggers;
            break; // break the infinite loop to write and close the current file
          }
          tm = 0;
        }
      }
      num_files += 1;
      cerr << "---> " << num_triggers_in_files << " triggers written in current files" << endl;
      unsigned int i = 0;
      for (VME::TDCCollection::const_iterator atdc=tdcs.begin(); atdc!=tdcs.end(); atdc++, i++) {
        if (out_file[i].is_open()) out_file[i].close();
        vme->SendOutputFile(atdc->first, fh.spill_id);
      }
    } while (true);
  } catch (Exception& e) {
    if (e.ErrorNumber()==TDC_ACQ_STOP) {
      unsigned int i = 0;
      try {
        VME::TDCCollection tdcs = vme->GetTDCCollection();
        for (VME::TDCCollection::const_iterator atdc=tdcs.begin(); atdc!=tdcs.end(); atdc++, i++) {
          if (out_file[i].is_open()) out_file[i].close();
          vme->SendOutputFile(atdc->first, fh.spill_id);
        }
  
        time_t t_end = time(0);
        double nsec_tot = difftime(t_end, t_beg), nsec = fmod(nsec_tot,60), nmin = (nsec_tot-nsec)/60.;
        cerr << endl << "*** Acquisition stopped! ***" << endl
             << "Local time: " << asctime(localtime(&t_end))
             << "Total acquisition time: " << difftime(t_end, t_beg) << " seconds"
             << " (" << nmin << " min " << nsec << " sec)"
             << endl;

        cerr << endl << "Acquired ";
        for (unsigned int i=0; i<num_tdc; i++) { if (i>0) cerr << " / "; cerr << num_events[i]; }
        cerr << " words in " << num_files << " files for " << num_all_triggers << " triggers in this run" << endl;
    
        ostringstream os;
        os << "Acquired ";
        for (unsigned int i=0; i<num_tdc; i++) { if (i>0) os << " / "; os << num_events[i]; }
        os << " words in " << num_files << " files for " << num_all_triggers << " triggers in this run" << endl
           << "Total acquisition time: " << difftime(t_end, t_beg) << " seconds"
           << " (" << nmin << " min " << nsec << " sec)";
        if (vme->UseSocket()) vme->Send(Exception(__PRETTY_FUNCTION__, os.str(), Info));
      
        delete vme;
      } catch (Exception& e) { e.Dump(); }
      return 0;
    }
    e.Dump();
    if (vme->UseSocket()) vme->Send(e);
    return -1;
  }
    
  return 0;
}
