#ifndef VME_FPGAUnitV1495_h
#define VME_FPGAUnitV1495_h

#include "VME_GenericBoard.h"

#include <unistd.h>

namespace VME
{
  enum FPGAUnitV1495Register {
    // User-defined registers
    kV1495ScalerCounter     = 0x100c,
    kV1495UserFWRevision    = 0x1014,
    kV1495TDCBoardInterface = 0x1018,
    kV1495ClockSettings     = 0x101c,
    kV1495Control           = 0x1020,
    kV1495TriggerSettings   = 0x1024,
    kV1495OutputSettings    = 0x1028,
    // CAEN board registers
    kV1495GeoAddress        = 0x8008,
    kV1495UserFPGAFlashMem  = 0x8014,
    kV1495UserFPGAConfig    = 0x8016,
    kV1495ModuleReset       = 0x800a,
    kV1495FWRevision        = 0x800c,
    kV1495ConfigurationROM  = 0x8100,
    kV1495OUI2              = 0x8124,
    kV1495OUI1              = 0x8128,
    kV1495OUI0              = 0x812c,
    kV1495Board2            = 0x8134,
    kV1495Board1            = 0x8138,
    kV1495Board0            = 0x813c,
    kV1495HWRevision3       = 0x8140,
    kV1495HWRevision2       = 0x8144,
    kV1495HWRevision1       = 0x8148,
    kV1495HWRevision0       = 0x814c,
    kV1495SerNum0           = 0x8180,
    kV1495SerNum1           = 0x8184
  };

  /**
   * User-defined control word to be propagated to the
   * CAEN V1495 board firmware.
   * \author Laurent Forthomme <laurent.forthomme@cern.ch>
   * \date 27 Jun 2015
   */
  class FPGAUnitV1495Control
  {
    public:
      inline FPGAUnitV1495Control(uint32_t word) : fWord(word) {;}
      inline virtual ~FPGAUnitV1495Control() {;}

      inline void Dump() const {
        std::ostringstream os;
        os << "Raw control word: ";
        for (int i=15; i>=0; i--) {
          os << ((fWord>>i)&0x1);
          if (i%4==0) os << " ";
        }
        PrintInfo(os.str());
      }
      inline uint32_t GetWord() const { return fWord; }
      
      enum ClockSource { InternalClock=0x0, ExternalClock=0x1 };
      /**
       * \brief Get the clock source
       */
      inline ClockSource GetClockSource() const { return static_cast<ClockSource>(GetBit(0)); }
      /**
       * \brief Switch between internal and external clock source
       */
      inline void SetClockSource(const ClockSource& cs) { SetBit(0, cs); }
      
      enum TriggerSource { InternalTrigger=0x0, ExternalTrigger=0x1 };
      /**
       * \brief Get the trigger source
       */
      inline TriggerSource GetTriggerSource() const { return static_cast<TriggerSource>(GetBit(1)); }
      /**
       * \brief Switch between internal and external trigger source
       */
      inline void SetTriggerSource(const TriggerSource& cs) { SetBit(1, cs); }

      inline bool GetScalerStatus() const { return GetBit(2); }
      inline void SetScalerStatus(bool start=true) { SetBit(2, start); }
      inline void SetScalerReset(bool reset=true) { SetBit(3, reset); }

      enum SignalSource { InternalSignal=0x0, ExternalSignal=0x1 };
      inline SignalSource GetSignalSource(unsigned short map_id) const { return static_cast<SignalSource>(GetBit(4+map_id)); }
      inline void SetSignalSource(unsigned short map_id, const SignalSource& s) { SetBit(4+map_id, s); }

    private:
      inline bool GetBit(unsigned short id) const { return static_cast<bool>((fWord>>id)&0x1); }
      inline void SetBit(unsigned short id, unsigned short value=0x1) {
        if (value==GetBit(id)) return;
        unsigned short sign = (value==0x0) ? -1 : 1; fWord += sign*(0x1<<id);
      }
      uint32_t fWord;
  };

  /**
   * Handler for the multi-purposes FPGA unit (CAEN V1495)
   * \author Laurent Forthomme <laurent.forthomme@cern.ch>
   * \date 25 Jun 2015
   */
  class FPGAUnitV1495 : public GenericBoard<FPGAUnitV1495Register,cvA32_U_DATA>
  {
    public:
      FPGAUnitV1495(int32_t bhandle, uint32_t baseaddr);
      ~FPGAUnitV1495();

      unsigned short GetCAENFirmwareRevision() const;
      unsigned short GetUserFirmwareRevision() const;
      unsigned int GetHardwareRevision() const;
      unsigned short GetSerialNumber() const;
      unsigned short GetGeoAddress() const;
      void CheckBoardVersion() const;
      void ResetFPGA() const;

      void DumpFWInformation() const;

      enum TDCBits { kReset=0x1, kTrigger=0x2, kClear=0x4 };
      /**
       * \brief Set a pattern of bits to be sent to all TDCs through the ECL mezzanine
       */
      void SetTDCBits(unsigned short bits) const;
      /**
       * \brief Send a pulse to TDCs' front panel
       * \param[in] bits The pattern to send (3 bits)
       * \param[in] time_us Pulse width (in us)
       */
      void PulseTDCBits(unsigned short bits, unsigned int time_us=10) const;
      /**
       * \brief Retrieve the current bits sent to TDCs' front panel
       * \return A 3-bit word PoI
       */
      unsigned short GetTDCBits() const;

      /**
       * \brief Retrieve the user-defined control word
       */
      FPGAUnitV1495Control GetControl() const;
      /**
       * \brief Set the user-defined control word
       */
      void SetControl(const FPGAUnitV1495Control& control) const;

      /**
       * \brief Set the internal clock period
       * \param[in] period Clock period (in units of 25 ns)
       */
      void SetInternalClockPeriod(uint32_t period) const;
      /**
       * \brief Retrieve the internal clock period
       * \return Clock period (in units of 25 ns)
       */
      uint32_t GetInternalClockPeriod() const;

      /**
       * \brief Set the internal trigger period
       * \param[in] period Trigger period (in units of 50 ns)
       */
      void SetInternalTriggerPeriod(uint32_t period) const;
      /**
       * \brief Retrieve the internal trigger period
       * \return Trigger period (in units of 50 ns)
       */
      uint32_t GetInternalTriggerPeriod() const;

      uint32_t GetOutputPulser() const;
      void ClearOutputPulser() const;
      void SetOutputPulser(unsigned short id, bool enable=true) const;
      void SetOutputPulserPOI(uint32_t poi) const;

      /// Start the inner triggers counter
      void StartScaler();
      /// Stop the inner triggers counter
      void StopScaler();
      //void PauseScaler() const;
      /// Return the inner triggers counter value
      uint32_t GetScalerValue() const;
 
    private:
      bool fScalerStarted;
  };
}

#endif
