#ifndef NIM_HVModuleN470_h
#define NIM_HVModuleN470_h

#include "VME_CAENETControllerV288.h"

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
  class HVModuleN470
  {
    public:
      HVModuleN470(uint16_t addr, VME::CAENETControllerV288& cont);
      inline ~HVModuleN470() {;}

      unsigned short GetModuleId();
      unsigned short GetFWRevision();

    private:
      /**
       * Read a vector of 16-bit words in the register
       * \brief Read in register
       * \param[in] addr register
       * \param[out] vector of data words
       */
      void ReadRegister(const HVModuleN470Opcodes& reg, std::vector<uint16_t>* data, unsigned int num_words=1);
      /**
       * Write a 16-bit word in the register
       * \brief Write on register
       * \param[in] addr register
       * \param[out] data word
       */
      void WriteRegister(const HVModuleN470Opcodes& reg, const uint16_t& data);

      VME::CAENETControllerV288 fController;
      uint16_t fAddress;
  };
}

#endif
