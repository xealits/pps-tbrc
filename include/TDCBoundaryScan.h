#ifndef TDCBoundaryScan_h
#define TDCBoundaryScan_h

#include "TDCRegister.h"

/**
 * 
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 24 Apr 2015
 * \ingroup HPTDC
 */
class TDCBoundaryScan : public TDCRegister
{
  public:
    inline TDCBoundaryScan() : TDCRegister(TDC_BS_BITS_NUM) {
      SetConstantValues(); }
    inline TDCBoundaryScan(const TDCBoundaryScan& bs) :
      TDCRegister(TDC_BS_BITS_NUM, bs) { SetConstantValues(); }
    
    inline void SetConstantValues() {
      
    }
  private:
    static const bit kTokenOut = 0;
    static const bit kStrobeOut = 1;
    static const bit kSerialOut = 2;
    static const bit kTest = 3;
    static const bit kError = 4;
    static const bit kDataReady = 5;
    static const bit kParallelEnable = 6;
    static const bit kParallelDataOut = 7;
    static const bit kEncodedControl = 39;
    static const bit kTrigger = 40;
    static const bit kEventReset = 41;
    static const bit kBunchReset = 42;
    static const bit kGetData = 43;
    static const bit kSerialBypassIn = 44;
    static const bit kSerialIn = 45;
    static const bit kTokenBypassIn = 46;
    static const bit kTokenIn = 47;
    static const bit kReset = 48;
    static const bit kAuxClock = 49;
    static const bit kClk = 50;
    static const bit kHit = 51;
};

#endif
