#ifndef VMEReader_h
#define VMEReader_h

#include "Client.h"
#include "VME_BridgeVx718.h"
#include "VME_TDCV1x90.h"
#include "VME_TDCEvent.h"

class VMEReader : public Client
{
  public:
    VMEReader(const char *device, unsigned int type);
    virtual ~VMEReader();
    
    VME::TDCV1x90* GetTDC() { return fTDC; }
    unsigned int GetRunNumber();

  private:
    VME::BridgeVx718* fBridge;
    VME::TDCV1x90* fTDC;
    
};

#endif
