#include "VME_CAENETControllerV288.h"

namespace VME
{
  CAENETControllerV288::CAENETControllerV288(int32_t bhandle, uint32_t baseaddr) :
    GenericBoard<CAENETControllerV288Register,cvA24_U_DATA>(bhandle, baseaddr)
  {;}

  void
  operator<<(uint16_t& word, const CAENETControllerV288& cnt)
  {
    try { cnt.WriteRegister(kV288DataBuffer, word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to fill the buffer with an additional word", JustWarning);
    }
  }

  uint16_t&
  operator>>(uint16_t& word, const CAENETControllerV288& cnt)
  {
    try { cnt.ReadRegister(kV288DataBuffer, &word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve an additional word from the buffer", JustWarning);
    }
    return word;
  }

  void
  CAENETControllerV288::Send() const
  {
  }

  std::vector<uint16_t>
  CAENETControllerV288::Receive() const
  {
  }
}
