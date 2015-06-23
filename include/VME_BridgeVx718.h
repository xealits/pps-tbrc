/* Interface for CAEN Vx718 VME Bridge */

#ifndef BRIDGEVx718_H 
#define BRIDGEVx718_H

#include "CAENVMElib.h"
#include <iostream>
#include <sstream>
#include <map>

#include "Exception.h"

namespace VME
{
  /// Compatible bridge types
  enum BridgeType { CAEN_V1718, CAEN_V2718 };

  class BridgeVx718Status
  {
    public:
      inline BridgeVx718Status(uint16_t word) : fWord(word) {;}
      inline virtual ~BridgeVx718Status() {;}
      
      inline void Dump() const {
        std::ostringstream os;
        PrintInfo(os.str());
      }

      inline bool GetSystemReset() const { return static_cast<bool>(fWord&0x1); }
      inline bool GetSystemControl() const { return static_cast<bool>((fWord>>1)&0x1); }
      inline bool GetDTACK() const { return static_cast<bool>((fWord>>4)&0x1); }
      inline bool GetBERR() const { return static_cast<bool>((fWord>>5)&0x1); }
      inline bool GetDipSwitch(unsigned int sw) const {
        if (sw>5) {
          std::ostringstream os; os << "Trying to get the value of a dip switch outside allowed range [0:4]! (" << sw << ")";
          throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
        }
        return static_cast<bool>((fWord>>(8+sw))&0x1);
      }
      inline bool GetUSBType() const { return static_cast<bool>((fWord>>15)&0x1); }

    private:
      uint16_t fWord;
  };

  class BridgeVx718Control
  {
    public:
      inline BridgeVx718Control(uint16_t word) : fWord(word) {;}
      inline virtual ~BridgeVx718Control() {;}

      /**
       * \brief Arbiter type
       * \return true if "Round Robin", else fixed priority
       */
      inline bool GetArbiterType() const { return static_cast<bool>((fWord>>1)&0x1); }
      /**
       * \brief Requester type
       * \return true if demand, else fair
       */
      inline bool GetRequesterType() const { return static_cast<bool>((fWord>>2)&0x1); }
      /**
       * \brief Release type
       * \return true if release on request, else release when done
       */
      inline bool GetReleaseType() const { return static_cast<bool>((fWord>>3)&0x1); }
      inline unsigned int GetBusReqLevel() const { return static_cast<unsigned int>((fWord>>4)&0x3); }
      inline bool GetInterruptReq() const { return static_cast<bool>((fWord>>6)&0x1); }
      inline bool GetSysRes() const { return static_cast<bool>((fWord>>7)&0x1); }
      /**
       * \brief VME bus timeout
       * \return true if 1400 us, else 50 us
       */
      inline bool GetBusTimeout() const { return static_cast<bool>((fWord>>8)&0x1); }
      /**
       * \brief Address Increment
       * \return true if enabled, else false (FIFO mode)
       */
      inline bool GetAddressIncrement() const { return static_cast<bool>((fWord>>9)&0x1); }

      
    private:
      uint16_t fWord;
  };
  
  /**
   * This class initializes the CAEN V1718 VME bridge in order to control the crate.
   * \brief class defining the VME bridge
   * \author Laurent Forthomme <laurent.forthomme@cern.ch>
   * \author Bob Velghe <bob.velghe@cern.ch>
   * \date Jun 2010
   */
  class BridgeVx718
  {
    public:
      /* device : /dev/xxx */
      /**
       * Bridge class constructor
       * \brief Constructor
       * \param[in] device Device identifier on the VME crate
       * \param[in] type Device type (1718/2718)
       */
      BridgeVx718(const char *device, BridgeType type);
      /**
       * Bridge class destructor
       * \brief Destructor
       */
      ~BridgeVx718();

      /**
       * Gives bhandle value
       * \brief Gets bhandle
       * \return bhandle value
       */ 
      inline int32_t GetHandle() const { return fHandle; }
      void CheckConfiguration() const;

      /**
       * \brief Set and control the output lines
       */
      void OutputConf(CVOutputSelect output) const;
      void OutputOn(CVOutputSelect output) const;
      void OutputOff(CVOutputSelect output) const;

      /**
       * \brief Set and read the input lines
       */
      void InputConf(CVInputSelect input) const;
      void InputRead(CVInputSelect input) const;

      void StartPulser(double period, double width, unsigned char num_pulses=0) const;
      void StopPulser() const;

    private:
      void WriteRegister(CVRegisters addr, const uint16_t& data) const;
      void WriteRegister(CVRegisters addr, const uint32_t& data) const;
      void ReadRegister(CVRegisters addr, uint16_t* data) const;
      void ReadRegister(CVRegisters addr, uint32_t* data) const;

      /// Device handle
      int32_t fHandle;
      uint32_t fBaseAddr;
  };
}

#endif
