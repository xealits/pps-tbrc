#ifndef VMEReader_h
#define VMEReader_h

#include "Client.h"
#include "VME_BridgeVx718.h"
#include "VME_TDCV1x90.h"
#include "VME_TDCEvent.h"

/**
 * VME reader object to fetch events on a HPTDC board
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 4 May 2015
 */
class VMEReader : public Client
{
  public:
    /**
     * \param[in] device Path to the device (/dev/xxx)
     * \param[in] type Bridge model
     * \param[in] on_socket Are we trying to connect through the socket?
     */
    VMEReader(const char *device, VME::BridgeType type, bool on_socket=true);
    virtual ~VMEReader();
    
    /// Return the TDC
    void AddTDC(uint32_t address);
    VME::TDCV1x90* GetTDC(uint32_t address) {
      return fTDCCollection[address];
    }
    /// Ask the socket master a run number
    unsigned int GetRunNumber();
    
    void Abort();

  private:
    typedef std::map<uint32_t,VME::TDCV1x90*> TDCCollection;
    VME::BridgeVx718* fBridge;
    TDCCollection fTDCCollection;
    bool fOnSocket;
};

#endif
