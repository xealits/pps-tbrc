#ifndef TDC_h
#define TDC_h

#include "USBHandler.h"

#include "TDCSetup.h"
#include "TDCControl.h"
#include "TDCBoundaryScan.h"
#include "TDCStatus.h"

#include "TDCConstants.h"

/**
 * \brief HPTDC object
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 27 Apr 2015 
 */
class TDC
{
  public:
    inline TDC(USBHandler* h) : fUSB(h) {;}
    ~TDC() {;}
    
    /// Submit the HPTDC setup word as a TDCSetup object
    inline void SetSetupRegister(const TDCSetup& c) { fSetup = c; }
    /// Retrieve the HPTDC setup word as a TDCSetup object
    inline TDCSetup GetSetupRegister() { return fSetup; }
    
    void ReadStatus() {
      fStatus = ReadRegister<TDCStatus>(TDC_STATUS_REGISTER);
    }
    
  private:
    /// Set the setup word to the HPTDC internal setup register
    void SendConfiguration();
    /// Read the setup word from the HPTDC internal setup register
    void ReadConfiguration();
    /// Write one register content on the HPTDC inner memory
    template<class T> void WriteRegister(unsigned int r, const T& v);
    /// Retrieve one register content from the HPTDC inner memory
    template<class T> T ReadRegister(unsigned int r);
    
    USBHandler* fUSB;
    
    TDCSetup fSetup;
    TDCControl fControl;
    TDCBoundaryScan fBS;
    TDCStatus fStatus;

};

#endif
