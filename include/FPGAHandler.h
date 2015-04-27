#ifndef FPGAHandler_h
#define FPGAHandler_h

#include "Client.h"
#include "USBHandler.h"

#include "TDCSetup.h"
#include "TDCControl.h"
#include "TDCBoundaryScan.h"
#include "TDCStatus.h"

#include "TDCConstants.h"

#include <fstream>

/**
 * General header to store in each collected data file for offline readout. It
 * enable any reader to retrieve the run/spill number, as well as the HPTDC
 * configuration during data collection.
 * \brief Header to the output files
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 14 Apr 2015 
 */
struct file_header_t {
  uint32_t magic;
  uint32_t run_id;
  uint32_t spill_id;
  TDCSetup config;
};

/**
 * Main driver for a homebrew FPGA designed for the timing detectors' HPTDC
 * chip readout.
 * \brief Driver for timing detectors' FPGA readout
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 14 Apr 2015
 */
class FPGAHandler : public Client, private USBHandler
{
  public:
    /// Bind to a FPGA through the USB protocol, and to the socket
    FPGAHandler(int port, const char* dev);
    virtual ~FPGAHandler();
    
    /// Open an output file to store header/HPTDC events
    void OpenFile();
    /// Close a previously opened output file used to store header/HPTDC events
    void CloseFile();
    /// Retrieve the file name used to store data collected from the FPGA
    inline std::string GetFilename() const { return fFilename; }
    
    /// Submit the HPTDC setup word as a TDCSetup object
    inline void SetConfiguration(const TDCSetup& c) {
      fTDCSetup=c;
      SendConfiguration();
    }
    /// Retrieve the HPTDC setup word as a TDCSetup object
    inline TDCSetup GetConfiguration() {
      ReadConfiguration();
      return fTDCSetup;
    }
    
    bool ErrorState();
    
    void ReadBuffer();
    /// Socket actor type retrieval method
    inline SocketType GetType() const { return DETECTOR; }

  private:
    /// Set the setup word to the HPTDC internal setup register
    void SendConfiguration();
    /// Read the setup word from the HPTDC internal setup register
    void ReadConfiguration();
    
    void SetRegister(const TDCControl::RegisterName& r, unsigned int v);
    unsigned int ReadRegister(const TDCControl::RegisterName& r);
    
    void ReadBoundaryScanRegister();
    
    std::string fFilename;
    std::ofstream fOutput;
    bool fIsFileOpen;
    bool fIsTDCInReadout;
    
    TDCSetup fTDCSetup;
    TDCControl fTDCControl;
    TDCBoundaryScanRegister fTDCBSR;
    TDCStatus fTDCStatus;    
};

#endif
