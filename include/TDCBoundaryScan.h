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
    inline TDCBoundaryScanRegister() : TDCRegister(83) {
    }
    inline virtual ~TDCBoundaryScanRegister() {;}
};

#endif
