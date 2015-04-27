#ifndef TDCStatus_h
#define TDCStatus_h

#include "TDCRegister.h"

/**
 * 
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 27 Apr 2015
 * \ingroup HPTDC
 */
class TDCStatus : public TDCRegister
{
  public:
    inline TDCStatus() : TDCRegister(TDC_STATUS_BITS_NUM) { SetConstantValues(); }
    inline TDCStatus(const TDCStatus& s) :
      TDCRegister(TDC_STATUS_BITS_NUM, s) { SetConstantValues(); }
    
    inline void SetConstantValues() {}
    
  private:
    static const bit kError = 0;
    static const bit kHaveToken = 11;
    static const bit kReadoutFIFOOccupancy = 12;
    static const bit kReadoutFIFOFull = 20;
    static const bit kReadoutFIFOEmpty = 21;
    static const bit kL1Occupancy = 22;
    static const bit kTriggerFIFOOccupancy = 54;
    static const bit kTriggerFIFOFull = 58;
    static const bit kTriggerFIFOEmpty = 59;
    static const bit kDLLLock = 60;
};

#endif
