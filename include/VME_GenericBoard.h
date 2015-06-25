#ifndef VME_GenericBoard_h
#define VME_GenericBoard_h

#include <stdint.h>
#include <sstream>

#include "CAENVMElib.h"
#include "CAENVMEtypes.h"
#include "CAENVMEoslib.h"

#include "Exception.h"

namespace VME
{
  template<class Register, CVAddressModifier am>
  class GenericBoard
  {
    public:
      inline GenericBoard(int32_t bhandle, uint32_t baseaddr) : fHandle(bhandle), fBaseAddr(baseaddr) {;}
      inline virtual ~GenericBoard() {;}

    protected:
      /**
       * Write a 16-bit word in the register
       * \brief Write on register
       * \param[in] addr register
       * \param[in] data word
       */
      inline void WriteRegister(const Register& reg, const uint16_t& data) const {
        uint32_t address = fBaseAddr+reg;
        uint16_t* fdata = new uint16_t; *fdata = data;
        std::cout << __PRETTY_FUNCTION__ << " -> address modifier = " << am << std::endl;
        CVErrorCodes out = CAENVME_WriteCycle(fHandle, address, fdata, am, cvD16);
        if (out!=cvSuccess) {
          std::ostringstream o;
          o << "Impossible to write register at 0x" << std::hex << reg << "\n\t"
            << "Addressing mode: 0x" << std::hex << am << "\n\t"
            << "Base address: 0x" << std::hex << fBaseAddr << "\n\t"
            << "CAEN error: " << CAENVME_DecodeError(out);
          throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
        }
      }
      /**
       * Write a 32-bit word in the register
       * \brief Write on register
       * \param[in] addr register
       * \param[in] data word
       */
      inline void WriteRegister(const Register& reg, const uint32_t& data) const {
        uint32_t address = fBaseAddr+reg;
        uint32_t* fdata = new uint32_t; *fdata = data;
        std::cout << __PRETTY_FUNCTION__ << " -> address modifier = " << am << std::endl;
        CVErrorCodes out = CAENVME_WriteCycle(fHandle, address, fdata, am, cvD32);
        if (out!=cvSuccess) {
          std::ostringstream o;
          o << "Impossible to write register at 0x" << std::hex << reg << "\n\t"
            << "Addressing mode: 0x" << std::hex << am << "\n\t"
            << "Base address: 0x" << std::hex << fBaseAddr << "\n\t"
            << "CAEN error: " << CAENVME_DecodeError(out);
          throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
        }
      }
      /**
       * Read a 16-bit word in the register
       * \brief Read on register
       * \param[in] addr register
       * \param[out] data word
       */
      inline void ReadRegister(const Register& reg, uint16_t* data) const {
        uint32_t address = fBaseAddr+reg;
        std::cout << __PRETTY_FUNCTION__ << " -> address modifier = " << am << std::endl;
        CVErrorCodes out = CAENVME_ReadCycle(fHandle, address, data, am, cvD16);
        if (out!=cvSuccess) {
          std::ostringstream o;
          o << "Impossible to read register at 0x" << std::hex << reg << "\n\t"
            << "Addressing mode: 0x" << std::hex << am << "\n\t"
            << "Base address: 0x" << std::hex << fBaseAddr << "\n\t"
            << "CAEN error: " << CAENVME_DecodeError(out);
          throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
        }
      }
      /**
       * Read a 32-bit word in the register
       * \brief Read on register
       * \param[in] addr register
       * \param[out] data word
       */
      inline void ReadRegister(const Register& reg, uint32_t* data) const {
        uint32_t address = fBaseAddr+reg;
        std::cout << __PRETTY_FUNCTION__ << " -> address modifier = " << am << std::endl;
        CVErrorCodes out = CAENVME_ReadCycle(fHandle, address, data, am, cvD32);
        if (out!=cvSuccess) {
          std::ostringstream o;
          o << "Impossible to read register at 0x" << std::hex << reg << "\n\t"
            << "Addressing mode: 0x" << std::hex << am << "\n\t"
            << "Base address: 0x" << std::hex << fBaseAddr << "\n\t"
            << "CAEN error: " << CAENVME_DecodeError(out);
          throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
        }
      }
      int32_t fHandle;
      uint32_t fBaseAddr;
  };
}

#endif
