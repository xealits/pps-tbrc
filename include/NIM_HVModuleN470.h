#ifndef NIM_HVModuleN470_h
#define NIM_HVModuleN470_h

#include "VME_CAENETControllerV288.h"
#define NUM_CHANNELS 4

namespace NIM
{
  enum HVModuleN470Opcodes {
    kN470GeneralInfo       = 0x00, // R
    kN470MonStatus         = 0x01, // R
    kN470OperationalParams = 0x02, // R
    kN470V0Value           = 0x03, // W
    kN470I0Value           = 0x04, // W
    kN470V1Value           = 0x05, // W
    kN470I1Value           = 0x06, // W
    kN470TripValue         = 0x07, // W
    kN470RampUpValue       = 0x08, // W
    kN470RampDownValue     = 0x09, // W
    kN470ChannelOn         = 0x0a, // W
    kN470ChannelOff        = 0x0b, // W
    kN470KillAllChannels   = 0x0c, // W
    kN470ClearAlarm        = 0x0d, // W
    kN470EnableFrontPanel  = 0x0e, // W
    kN470DisableFrontPanel = 0x0f, // W
    kN470TTLLevel          = 0x10, // W
    kN470NIMLevel          = 0x11  // W
  };

  /**
   * \brief General monitoring values for the HV power supply
   * \author Laurent Forthomme <laurent.forthomme@cern.ch>
   * \date 24 Jul 2015
   */
  class HVModuleN470Values
  {
    public:
      inline HVModuleN470Values(std::vector<unsigned short> vals) : fValues(vals) {;}
      inline ~HVModuleN470Values() {;}

      inline void Dump() const {
        std::ostringstream os;
        os << "Monitoring values:" << "\n\t"
           << "  Vmon = " << Vmon() << " V" << "\n\t"
           << "  Imon = " << Imon() << " uA" << "\n\t"
           << "  Vmax = " << Vmax() << " V" << "\n\t"
           << "  Individual channels status:" << "\n\t";
        for (unsigned int i=0; i<NUM_CHANNELS; i++) {
          os << "===== Channel " << i << " =====" << "\n\t"
             << GetChannelStatus(i);
          if (i<3) os << "\n\t";
        }
        PrintInfo(os.str());
      }

      inline unsigned short Vmon() const { return fValues.at(0); }
      inline unsigned short Imon() const { return fValues.at(1); }
      inline unsigned short Vmax() const { return fValues.at(2); }

      class ChannelStatus {
        public:
          inline ChannelStatus(unsigned short word) : fWord(word) {;}
          inline ~ChannelStatus() {;}

          inline bool Enabled() const { return (fWord&0x1); }
          inline bool OVC() const { return ((fWord>>1)&0x1); }
          inline bool OVV() const { return ((fWord>>2)&0x1); }
          inline bool UNV() const { return ((fWord>>3)&0x1); }
          inline bool Trip() const { return ((fWord>>4)&0x1); }
          inline bool RampUp() const { return ((fWord>>5)&0x1); }
          inline bool RampDown() const { return ((fWord>>6)&0x1); }
          inline bool MaxV() const { return ((fWord>>7)&0x1); }
          inline bool Polarity() const { return ((fWord>>8)&0x1); }
          inline bool Vsel() const { return ((fWord>>9)&0x1); }
          inline bool Isel() const { return ((fWord>>10)&0x1); }
          inline bool Kill() const { return ((fWord>>11)&0x1); }
          inline bool HVEnabled() const { return ((fWord>>12)&0x1); }
          enum SignalStandard { NIM=0x0, TTL=0x1 };
          inline SignalStandard Standard() const { return static_cast<SignalStandard>((fWord>>13)&0x1); }
          inline bool NonCalibrated() const { return ((fWord>>14)&0x1); }
          inline bool Alarm() const { return ((fWord>>15)&0x1); }

          inline friend std::ostream& operator<<(std::ostream& os, const ChannelStatus& cs) {
            os << "Channel enabled? " << cs.Enabled() << " with polarity " << cs.Polarity() << "\n\t"
               << "OVC=" << cs.OVC() << ", OVV=" << cs.OVV() << ", UNV=" << cs.UNV() << "\n\t"
               << "Trip? " << cs.Trip() << ", Kill? " << cs.Kill() << ", Alarm? " << cs.Alarm() << "\n\t"
               << "Ramp up? " << cs.RampUp() << " / down? " << cs.RampDown() << "\n\t"
               << "MaxV=" << cs.MaxV() << ", Vsel=" << cs.Vsel() << ", Isel=" << cs.Isel() << "\n\t"
               << "HV enabled? " << cs.HVEnabled();
            return os;
          }
          inline void Dump() const {
            std::ostringstream os; os << this;
            PrintInfo(os.str());
          }
        private:
          unsigned short fWord;
      };
      inline ChannelStatus GetChannelStatus(unsigned short ch_id) const {
        if (ch_id<0 or ch_id>=NUM_CHANNELS) return 0;
        return ChannelStatus(fValues.at(3+ch_id));
      }

    private:
      std::vector<unsigned short> fValues;
  };

  /**
   * \brief Single channel monitoring values for the HV power supply
   * \author Laurent Forthomme <laurent.forthomme@cern.ch>
   * \date 24 Jul 2015
   */
  class HVModuleN470ChannelValues
  {
    public:
      inline HVModuleN470ChannelValues(unsigned short ch_id, std::vector<unsigned short> vals) :
        fChannelId(ch_id), fValues(vals) {;}
      inline ~HVModuleN470ChannelValues() {;}
      
      inline void Dump() const {
        std::ostringstream os;
        os << "Individual channel values: channel " << fChannelId << "\n\t"
           << "Vmon/Imon = " << std::setw(4) << Vmon() << " V / " << std::setw(4) << Imon() << " uA\n\t"
           << "V0/I0 = " << std::setw(4) << V0() << " V / " << std::setw(4) << I0() << " uA\n\t"
           << "V1/I1 = " << std::setw(4) << V1() << " V / " << std::setw(4) << I1() << " uA\n\t"
           << "Trip = " << Trip() << "\n\t"
           << "Ramp up/down = " << RampUp() << " / " << RampDown() << "\n\t"
           << "Maximal V = " << MaxV() << " V";
        PrintInfo(os.str());
      }

      inline unsigned short ChannelStatus() const { return fValues.at(0); }
      inline unsigned short Vmon() const { return fValues.at(1); }
      inline unsigned short Imon() const { return fValues.at(2); }
      inline unsigned short V0() const { return fValues.at(3); }
      inline unsigned short I0() const { return fValues.at(4); }
      inline unsigned short V1() const { return fValues.at(5); }
      inline unsigned short I1() const { return fValues.at(6); }
      inline unsigned short Trip() const { return fValues.at(7); }
      inline unsigned short RampUp() const { return fValues.at(8); }
      inline unsigned short RampDown() const { return fValues.at(9); }
      inline unsigned short MaxV() const { return fValues.at(10); }

    private:
      unsigned short fChannelId;
      std::vector<unsigned short> fValues;
  };

  class HVModuleN470
  {
    public:
      HVModuleN470(uint16_t addr, VME::CAENETControllerV288& cont);
      inline ~HVModuleN470() {;}

      std::string GetModuleId() const;
      unsigned short GetFWRevision() const;
      HVModuleN470Values ReadMonitoringValues() const;
      HVModuleN470ChannelValues ReadChannelValues(unsigned short ch_id) const;

      void SetChannelV0(unsigned short ch_id, unsigned short v0) const;
      void SetChannelI0(unsigned short ch_id, unsigned short i0) const;
      void SetChannelV1(unsigned short ch_id, unsigned short v1) const;
      void SetChannelI1(unsigned short ch_id, unsigned short i1) const;

    private:
      /**
       * Read a vector of 16-bit words in the register
       * \brief Read in register
       * \param[in] addr register
       * \param[out] vector of data words
       */
      void ReadRegister(const HVModuleN470Opcodes& reg, std::vector<uint16_t>* data, unsigned int num_words=1) const;
      /**
       * Write a vector of 16-bit words in the register
       * \brief Write on register
       * \param[in] addr register
       * \param[out] data word
       */
      void WriteRegister(const HVModuleN470Opcodes& reg, const std::vector<uint16_t>& data) const;
      /**
       * Write a 16-bit word in the register
       * \brief Write on register
       * \param[in] addr register
       * \param[out] data word
       */
      void WriteRegister(const HVModuleN470Opcodes& reg, const uint16_t& data) const;

      VME::CAENETControllerV288 fController;
      uint16_t fAddress;
  };
}

#endif
