#ifndef VME_SignalGenerator_h
#define VME_SignalGenerator_h

#include <stdint.h>
#include <sstream>

#include "CAENVMElib.h"
#include "CAENVMEtypes.h"

#include "Exception.h"

namespace VME
{
  class SignalGenerator
  {
    public:
      enum ModuleRegister {
        kECLLevelWrite = 0x04,
        kNIMLevelWrite = 0x06,
        kNIMPulseWrite = 0x08,
        kNIMPulseRead  = 0x0A,
        kIdentifier    = 0xFA,
        kBoardInfo0    = 0xFC,
        kBoardInfo1    = 0xFE
      };
      SignalGenerator(int32_t bhandle, uint32_t baseaddr);
      inline ~SignalGenerator() {;}

      unsigned short GetSerialNumber() const;
      unsigned short GetModuleVersion() const;
      unsigned short GetModuleType() const;
      unsigned short GetManufacturerId() const;

    private:
      /**
       * Write a 16-bit word in the register
       * \brief Write on register
       * \param[in] addr register
       * \param[in] data word
       */
      void WriteRegister(const ModuleRegister& reg, const uint16_t&) const;
      /**
       * Read a 16-bit word in the register
       * \brief Read on register
       * \param[in] addr register
       * \param[out] data word
       */  
      void ReadRegister(const ModuleRegister& reg, uint16_t*) const;

      int32_t fHandle;
      uint32_t fBaseAddr;
  };
}

#endif
