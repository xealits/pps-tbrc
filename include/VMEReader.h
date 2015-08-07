#ifndef VMEReader_h
#define VMEReader_h

#include "Client.h"
#include "OnlineDBHandler.h"

#include "VME_BridgeVx718.h"
#include "VME_FPGAUnitV1495.h"
#include "VME_IOModuleV262.h"
#include "VME_CFDV812.h"
#include "VME_CAENETControllerV288.h"
#include "VME_TDCV1x90.h"

#include "NIM_HVModuleN470.h"

#include <map>
#include "tinyxml2.h"

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
     * \brief Load an XML configuration file
     */
    void ReadXML(const char* filename);
    inline void ReadXML(std::string filename) { ReadXML(filename.c_str()); }

    void NewRun() const;

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
    inline size_t GetNumTDC() const { return fTDCCollection.size(); }
    inline VME::TDCCollection GetTDCCollection() { return fTDCCollection; }

    void AddIOModule(uint32_t address);
    inline VME::IOModuleV262* GetIOModule() { return fSG; }
 
    /**
     * \brief Add a CFD to handle
     * \param[in] address 32-bit address of the CFD module on the VME bus
     * Create a new CFD handler for the VME bus
     */
    void AddCFD(uint32_t address);
    /**
     * \brief Get a CFD on the VME bus
     * Return a pointer to the CFD object, given its physical address on the VME bus
     */
    inline VME::CFDV812* GetCFD(uint32_t address) {
      if (fCFDCollection.count(address)==0) return 0;
      return fCFDCollection[address];
    }
    inline size_t GetNumCFD() const { return fCFDCollection.size(); }
    inline VME::CFDCollection GetCFDCollection() { return fCFDCollection; }

    /**
     * \brief Add a multi-purposes FPGA board (CAEN V1495) to the crate controller
     * \param[in] address 32-bit address of the TDC module on the VME bus
     */
    void AddFPGAUnit(uint32_t address);
    /// Return the pointer to the FPGA board connected to this controller (if any ; 0 otherwise)
    inline VME::FPGAUnitV1495* GetFPGAUnit() { return fFPGA; }

    /// Ask the socket master a run number
    unsigned int GetRunNumber() const;
    
    /// Start the bridge's pulse generator [faulty]
    inline void StartPulser(double period, double width, unsigned int num_pulses=0) {
      try {
        fBridge->StartPulser(period, width, num_pulses);
        fIsPulserStarted = true;
      } catch (Exception& e) { throw e; }
    }
    /// Stop the bridge's pulse generator [faulty]
    inline void StopPulser() {
      try {
        fBridge->StopPulser();
        fIsPulserStarted = false;
      } catch (Exception& e) { throw e; }
    }
    /// Send a single pulse to the output register/plug connected to TDC boards
    inline void SendPulse(unsigned short output=0) const {
      try {
        fBridge->SinglePulse(output);
      } catch (Exception& e) { e.Dump(); }
    }
    /// Send a clear signal to both the TDC boards
    inline void SendClear() const {
      if (!fFPGA) return;
      try {
        fFPGA->PulseTDCBits(VME::FPGAUnitV1495::kClear);
      } catch (Exception& e) { e.Dump(); }
    }

    /// Add a high voltage module (controlled by a VME-CAENET controller) to the DAQ
    void AddHVModule(uint32_t vme_address, uint16_t nim_address);
    /// Retrieve the NIM high voltage module
    inline NIM::HVModuleN470* GetHVModule() { return fHV; }

    /// Set the path to the output file where the DAQ is to write
    void SetOutputFile(uint32_t tdc_address, std::string filename);
    /// Return the path to the output file the DAQ is currently writing to
    inline std::string GetOutputFile(uint32_t tdc_address) {
      OutputFiles::iterator it = fOutputFiles.find(tdc_address);
      if (it==fOutputFiles.end())
        throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve output file", JustWarning);
      return it->second;
    }
    /// Send the path to the output file through the socket
    void SendOutputFile(uint32_t tdc_address) const;
    void BroadcastNewBurst(unsigned int burst_id) const;
    void BroadcastTriggerRate(unsigned int burst_id, unsigned long num_triggers) const;

    inline bool UseSocket() const { return fOnSocket; }
    /// Abort data collection for all modules on the bus handled by the bridge
    void Abort();

  private:
    /// The VME bridge object to handle
    VME::BridgeVx718* fBridge;
    /// A set of pointers to TDC objects indexed by their physical VME address
    VME::TDCCollection fTDCCollection;
    /// A set of pointers to CFD objects indexed by their physical VME address
    VME::CFDCollection fCFDCollection;
    /// Pointer to the VME input/output module object
    VME::IOModuleV262* fSG;
    /// Pointer to the VME general purpose FPGA unit object
    VME::FPGAUnitV1495* fFPGA;
    /// Pointer to the VME CAENET controller
    VME::CAENETControllerV288* fCAENET;
    /// Pointer to the NIM high voltage module (passing through the CAENET controller)
    NIM::HVModuleN470* fHV;
    /// Are we dealing with socket message passing?
    bool fOnSocket;
    /// Is the bridge's pulser already started?
    bool fIsPulserStarted;
    typedef std::map<uint32_t, std::string> OutputFiles;
    /// Path to the current output files the DAQ is writing to
    /// (indexed by the TDC id)
    OutputFiles fOutputFiles;
};

#endif
