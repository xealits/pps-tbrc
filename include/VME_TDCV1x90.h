#ifndef VMETDCV1x90_H 
#define VMETDCV1x90_H

//#define DEBUG
#include <iostream>
#include <iomanip>
#include <sstream>

#include <cmath>
#include <string>
#include <map>

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "CAENVMElib.h"
#include "CAENVMEoslib.h"
#include "CAENVMEtypes.h"

#include "VME_TDCV1x90Opcodes.h"
#include "TDCEvent.h"
#include "Exception.h"

namespace VME
{
  typedef enum {
    MATCH_WIN_WIDTH        = 0,
    WIN_OFFSET             = 1,
    EXTRA_SEARCH_WIN_WIDTH = 2,
    REJECT_MARGIN          = 3,
    TRIG_TIME_SUB          = 4,
  } trig_conf;

  typedef enum {
    r800ps = 0,
    r200ps = 1,
    r100ps = 2,
    r25ps  = 3,
  } trailead_edge_lsb;

  typedef enum {
    WRITE_OK      = 0, /*!< \brief Is the TDC ready for writing? */
    READ_OK       = 1, /*!< \brief Is the TDC ready for reading? */
  } micro_handshake;

  typedef enum {
    CONT_STORAGE,
    TRIG_MATCH,
  } acq_mode;

  typedef enum {
    PAIR      = 0,
    OTRAILING = 1,
    OLEADING  = 2,
    TRAILEAD  = 3,
  } det_mode;

  typedef enum {
    DATA_READY    = 0,
    ALM_FULL      = 1,
    FULL          = 2,
    TRG_MATCH     = 3,
    HEADER_EN     = 4,
    TERM_ON       = 5,
    ERROR0        = 6,
    ERROR1        = 7,
    ERROR2        = 8,
    ERROR3        = 9,
    BERR_FLAG     = 10,
    PURG          = 11,
    RES_1         = 12,
    RES_2         = 13,
    PAIRED        = 14,
    TRIGGER_LOST  = 15,
  } stat_reg;

  typedef enum {
    BERREN                           = 0,
    TERM                             = 1,
    TERM_SW                          = 2,
    EMPTY_EVENT                      = 3,
    ALIGN64                          = 4,
    COMPENSATION_ENABLE              = 5,
    TEST_FIFO_ENABLE                 = 6,
    READ_COMPENSATION_SRAM_ENABLE    = 7,
    EVENT_FIFO_ENABLE                = 8,
    EXTENDED_TRIGGER_TIME_TAG_ENABLE = 9,
  } ctl_reg;

  typedef enum {

    //Register OutputBuffer   (0x0000),
    Control                 = 0x1000, // D16 R/W
    Status                  = 0x1002, // D16 R
    InterruptLevel          = 0x100a, // D16 R/W
    InterruptVector         = 0x100c, // D16 R/W
    GeoAddress              = 0x100e, // D16 R/W
    MCSTBase                = 0x1010, // D16 R/W
    MCSTControl             = 0x1012, // D16 R/W
    ModuleReset             = 0x1014, // D16 W
    kSoftwareClear           = 0x1016, // D16 W
    EventCounter            = 0x101c, // D32 R
    EventStored             = 0x1020, // D16 R
    BLTEventNumber          = 0x1024, // D16 R/W
    FirmwareRev             = 0x1026, // D16 R
    Micro                   = 0x102e, // D16 R/W
    MicroHandshake          = 0x1030, // D16 R
    
    EventFIFO               = 0x1038, // D32 R
    EventFIFOStoredRegister = 0x103c, // D16 R
    EventFIFOStatusRegister = 0x103e, // D16 R  
    
    ROMOui2                 = 0x4024,
    ROMOui1                 = 0x4028,
    ROMOui0                 = 0x402c,
    
    ROMBoard2               = 0x4034,
    ROMBoard1               = 0x4038,
    ROMBoard0               = 0x403c,
    ROMRevis3               = 0x4040,
    ROMRevis2               = 0x4044,
    ROMRevis1               = 0x4048,
    ROMRevis0               = 0x404c,
    ROMSerNum1              = 0x4080,
    ROMSerNum0              = 0x4084,
    
  } mod_reg;


  struct glob_offs {
    uint16_t coarse;
    uint16_t fine;
  };

  // Event structure (one for each trigger)

  struct trailead_t {
    uint32_t event_count;
    int total_hits[16];
    // key   -> channel,
    // value -> measurement
    std::multimap<int32_t,int32_t> leading;
    std::multimap<int32_t,int32_t> trailing;
    uint32_t ettt;
  };

  /**
   * 
   * \author Laurent Forthomme <laurent.forthomme@cern.ch>
   * \author Bob Velghe <bob.velghe@cern.ch>
   * \date Jun 2010
   */
  class TDCV1x90
  {
    public:
      TDCV1x90(int32_t, uint32_t, acq_mode acqm=TRIG_MATCH, det_mode detm=TRAILEAD);
      ~TDCV1x90();
      void SetVerboseLevel(unsigned short verb=0) { fVerb=verb; }
      
      uint32_t GetModel();
      uint32_t GetOUI();
      uint32_t GetSerialNumber();
      void CheckConfiguration();
      
      void SetPoI(uint16_t);
      void SetLSBTraileadEdge(trailead_edge_lsb);
      void SetAcquisitionMode(acq_mode);
      bool SetTriggerMatching();
      bool IsTriggerMatching();
      bool SetContinuousStorage();
      void GetFirmwareRev();
      
      void SetGlobalOffset(uint16_t,uint16_t);
      glob_offs ReadGlobalOffset();
      
      void SetRCAdjust(int,uint16_t);
      uint16_t ReadRCAdjust(int);
      
      uint32_t GetEventCounter();
      uint16_t GetEventStored();
      
      void SetDetection(det_mode);
      det_mode ReadDetection();
      
      void SetTDCEncapsulation(bool);
      bool GetTDCEncapsulation();
      void SetTDCErrorMarks(bool);
      //bool GetTDCErrorMarks();
      
      void ReadResolution(det_mode);
      void SetPairModeResolution(int,int);

      void SetBLTEventNumberRegister(uint16_t);
      uint16_t GetBLTEventNumberRegister();
      
      void SetWindowWidth(uint16_t);
      void SetWindowOffset(int16_t);

      uint16_t ReadTrigConf(trig_conf);

      bool WaitMicro(micro_handshake);
      bool SoftwareClear();
      bool SoftwareReset();
      bool HardwareReset();
      
      bool GetStatusRegister(stat_reg);
      void SetStatusRegister(stat_reg,bool);
      bool GetCtlRegister(ctl_reg);
      void SetCtlRegister(ctl_reg,bool);
      
      void SetETTT(bool);
      bool GetETTT();
      
      TDCEventCollection GetEvents();

      //bool IsEventFIFOReady();
      void SetFIFOSize(uint16_t);
      void ReadFIFOSize();
      
      // Close/Clean everything before exit
      void abort();
      
      /**
       * Write a 16-bit word in the register
       * \brief Write on register
       * \param[in] addr register
       * \param[in] data word
       */
      void WriteRegister(mod_reg,uint16_t*);
      /**
       * Write a 32-bit word in the register
       * \brief Write on register
       * \param[in] addr register
       * \param[in] data word
       */
      void WriteRegister(mod_reg,uint32_t*);
      /**
       * Read a 16-bit word in the register
       * \brief Read on register
       * \param[in] addr register
       * \param[out] data word
       */  
      void ReadRegister(mod_reg,uint16_t*);
      /**
       * Read a 32-bit word in the register
       * \brief Read on register
       * \param[in] addr register
       * \param[out] data word
       */  
      void ReadRegister(mod_reg,uint32_t*);

    private:
      uint32_t fBaseAddr;
      int32_t fHandle;
      det_mode fDetMode;
      unsigned short fVerb;
      
      CVAddressModifier am; // Address modifier
      CVAddressModifier am_blt; // Address modifier (Block Transfert)
      
      uint32_t* fBuffer;
        
      det_mode detm;
      acq_mode acqm;
      
      bool outBufTDCHeadTrail;
      bool outBufTDCErr;
      bool outBufTDCTTT;
      
      uint32_t nchannels;
      bool gEnd;
      std::string pair_lead_res[8]; 
      std::string pair_width_res[16];
      std::string trailead_edge_res[4];

  };
}

#endif

