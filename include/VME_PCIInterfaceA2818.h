#ifndef VME_PCIInterfaceA2828_h
#define VME_PCIInterfaceA2818_h

#include "CAENVMEtypes.h"
#include <string>

namespace VME
{
  class PCIInterfaceA2818
  {
    public:
      inline PCIInterfaceA2818(const char* device) {
        int dev = atoi(device);
        CVErrorCodes ret = CAENVME_Init(cvA2818, 0x0, dev, &fHandle);
        if (ret!=cvSuccess) {
          std::ostringstream os;
          os << "Failed to initiate communication with" << "\n\t"
             << "the PCI/VME interface board" << "\n\t"
             << "CAEN error: " << CAENVME_DecodeError(ret);
          throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(ret));
        }
      }
      inline virtual ~PCIInterfaceA2818() {
        CAENVME_End(fHandle);
      }

      inline std::string GetFWRevision() const {
        char out[30];
        CVErrorCodes ret = CAENVME_BoardFWRelease(fHandle, out);
        if (ret!=cvSuccess) {
          std::ostringstream os;
          os << "Failed to retrieve the FW version" << "\n\t"
             << "CAEN error: " << CAENVME_DecodeError(ret);
          throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, CAEN_ERROR(ret));
        }
        return std::string(out);
      }

    private:
      int fHandle;
  };
}

#endif
