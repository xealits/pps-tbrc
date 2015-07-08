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
  if (gEnd==0) { cerr << endl << "[C-c] Trying a clean exit!" << endl;
    vme->Abort();
  }
  else if (gEnd>=5) { cerr << endl << "[C-c > 5 times] ... Forcing exit!" << endl;
    exit(0);
  }
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

  //VME::AcquisitionMode acq_mode = VME::CONT_STORAGE;
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

  try {
    bool with_socket = false;
    vme = new VMEReader("/dev/a2818_0", VME::CAEN_V2718, with_socket);
    
    fh.run_id = vme->GetRunNumber();
    
    vme->AddFPGAUnit(0x00ee0000);
    fpga = vme->GetFPGAUnit();
    if (!fpga) throw Exception(__PRETTY_FUNCTION__, "FPGA not detected!", Fatal);

    /*fpga->ResetFPGA();
    exit(0);*/
    cout << ">>> " << fpga->GetUserFirmwareRevision() << endl;

    //fpga->SetInternalClockPeriod(1); // in units of 25 ns
    fpga->SetInternalTriggerPeriod(400000); // in units of 25 ns

    fpga->DumpFWInformation();
    //exit(0);

    //VME::FPGAUnitV1495Control c = fpga->GetControl();
    //c.SetClockSource(VME::FPGAUnitV1495Control::ExternalClock);
    //c.SetClockSource(VME::FPGAUnitV1495Control::InternalClock);
    //c.SetTriggerSource(VME::FPGAUnitV1495Control::ExternalTrigger);
    //c.SetTriggerSource(VME::FPGAUnitV1495Control::InternalTrigger);
    //fpga->SetControl(c);

    fpga->PulseTDCBits(VME::FPGAUnitV1495::kReset|VME::FPGAUnitV1495::kClear);
    for (unsigned short i=0; i<16; i++) {
      //bool enabled = (i%2==0);
      bool enabled = true;
      fpga->SetOutputPulser(i, true, enabled);
    }
    //exit(0);
    // TDC configuration
    const uint32_t tdc_address[] = { 0x00aa0000, 0x00bb0000 }; // V1290A (32 ch., CERN)

    for (unsigned int i=0; i<num_tdc; i++) {
      vme->AddTDC(tdc_address[i]);
      tdc[i] = vme->GetTDC(tdc_address[i]);
      tdc[i]->SetVerboseLevel(1);
      tdc[i]->SetAcquisitionMode(acq_mode);
      tdc[i]->SetDetectionMode(det_mode);
      tdc[i]->SetDLLClock(VME::TDCV1x90::DLL_PLL_HighRes);
      //tdc[i]->SetETTT();

      /*tdc[i]->SetWindowWidth(4095);
      tdc[i]->SetWindowOffset(0);*/
    
      std::ostringstream filename;
      filename << "events_board" << i << ".dat";
      //filename << GenerateFileName(0);
      out_file[i].open(filename.str().c_str(), fstream::out | ios::binary );	
      if (!out_file[i].is_open()) {
        throw Exception(__PRETTY_FUNCTION__, "Error opening file", Fatal);
      }
      out_file[i].write((char*)&fh, sizeof(file_header_t));
   }
 
    std::string acqmode, detmode;
    switch (tdc[0]->GetAcquisitionMode()) {
      case VME::CONT_STORAGE: acqmode = "Continuous storage"; break;
      case VME::TRIG_MATCH: acqmode = "Trigger matching"; break;
      default:
        acqmode = "[Invalid mode]";
        throw Exception(__PRETTY_FUNCTION__, "Invalid acquisition mode!", Fatal);
    }
    switch (tdc[0]->GetDetectionMode()) {
      case VME::PAIR: detmode = "Pair measurement"; break;
      case VME::OLEADING: detmode = "Leading edge only"; break;
      case VME::OTRAILING: detmode = "Trailing edge only"; break;
      case VME::TRAILEAD: detmode = "Leading and trailing edges"; break;
    }

    t_beg = std::time(0);

    cerr << endl << "*** Ready for acquisition! ***" << endl
         << "Acquisition mode: " << acqmode << endl 
         << "Detection mode: " << detmode << endl 
         << "Local time: " << asctime(std::localtime(&t_beg));

    fpga->PulseTDCBits(VME::FPGAUnitV1495::kReset|VME::FPGAUnitV1495::kClear); // send a RST+CLR signal from FPGA to TDCs

    while (true) {
      for (unsigned int i=0; i<num_tdc; i++) {
        ec = tdc[i]->FetchEvents();
        if (ec.size()==0) continue; // no events were fetched
        for (VME::TDCEventCollection::const_iterator e=ec.begin(); e!=ec.end(); e++) {
          uint32_t word = e->GetWord();
          out_file[i].write((char*)&word, sizeof(uint32_t));
          //cout << hex << e->GetType() << endl;
          if (e->GetType()==VME::TDCEvent::TDCMeasurement) cout << e->GetChannelId() << endl;
        }
        num_events[i] += ec.size();
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
      cerr << " words in this run" << endl;
  
      delete vme;
      return 0;
    }
    e.Dump();
    return -1;
  }
    
  return 0;
}
