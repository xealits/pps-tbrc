#ifndef TDCBoundaryScanRegister_h
#define TDCBoundaryScanRegister_h

#include "TDCRegister.h"

class TDCBoundaryScanRegister : public TDCRegister
{
  public:
    inline TDCBoundaryScanRegister() : TDCRegister(83) {
    }
    inline virtual ~TDCBoundaryScanRegister() {;}
};

#endif
