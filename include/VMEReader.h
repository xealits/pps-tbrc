#ifndef VMEReader_h
#define VMEReader_h

#include "Client.h"
#include "VME_BridgeVx718.h"
#include "VME_FPGAUnitV1495.h"
#include "VME_IOModuleV262.h"
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
    
    /**
     * \brief Add a TDC to handle
     * \param[in] address 32-bit address of the TDC module on the VME bus
     * Create a new TDC handler for the VME bus
     */
    void AddTDC(uint32_t address);
    /**
     * \brief Get a TDC on the VME bus
     * Return a pointer to the TDC object, given its physical address on the VME bus
     */
    inline VME::TDCV1x90* GetTDC(uint32_t address) {
      if (fTDCCollection.count(address)==0) return 0;
      return fTDCCollection[address];
    }

    void AddIOModule(uint32_t address);
    inline VME::IOModuleV262* GetIOModule() { return fSG; }

    void AddFPGAUnit(uint32_t address);
    inline VME::FPGAUnitV1495* GetFPGAUnit() { return fFPGA; }

    /// Ask the socket master a run number
    unsigned int GetRunNumber();
    
    inline void StartPulser(double period, double width, unsigned int num_pulses=0) {
      try {
        fBridge->StartPulser(period, width, num_pulses);
        fIsPulserStarted = true;
      } catch (Exception& e) { throw e; }
    }
    inline void StopPulser() {
      try {
        fBridge->StopPulser();
        fIsPulserStarted = false;
      } catch (Exception& e) { throw e; }
    }
    inline void SendPulse(unsigned short output=0) const {
      try {
        fBridge->SinglePulse(output);
      } catch (Exception& e) { e.Dump(); }
    }
    inline void SendClear() const {
      if (!fFPGA) return;
      try {
        fFPGA->PulseTDCBits(VME::FPGAUnitV1495::kClear);
      } catch (Exception& e) { e.Dump(); }
    }

    /// Abort data collection for all modules on the bus handled by the bridge
    void Abort();

  private:
    /// Mapper from physical VME addresses to pointers to TDC objects
    typedef std::map<uint32_t,VME::TDCV1x90*> TDCCollection;
    /// The VME bridge object to handle
    VME::BridgeVx718* fBridge;
    /// A set of pointers to TDC objects indexed by their physical VME address
    TDCCollection fTDCCollection;
    /// Pointer to the VME input/output module object
    VME::IOModuleV262* fSG;
    /// Pointer to the VME general purpose FPGA unit object
    VME::FPGAUnitV1495* fFPGA;
    /// Are we dealing with socket message passing?
    bool fOnSocket;
    bool fIsPulserStarted;
};

#endif
