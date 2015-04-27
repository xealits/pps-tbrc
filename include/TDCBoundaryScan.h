#ifndef TDCBoundaryScanRegister_h
#define TDCBoundaryScanRegister_h

#include "TDCRegister.h"

/**
 * 
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 24 Apr 2015
 * \ingroup HPTDC
 */
class TDCBoundaryScanRegister : public TDCRegister
{
  public:
    inline TDCBoundaryScanRegister() : TDCRegister(TDC_BS_BITS_NUM) {
      SetConstantValues(); }
    inline TDCBoundaryScanRegister(const TDCBoundaryScanRegister& bs) :
      TDCRegister(TDC_BS_BITS_NUM, bs) { SetConstantValues(); }
    
    inline void SetConstantValues() {
      
    }
};

#endif
