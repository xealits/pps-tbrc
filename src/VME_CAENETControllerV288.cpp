#include "VME_CAENETControllerV288.h"

namespace VME
{
  CAENETControllerV288::CAENETControllerV288(int32_t bhandle, uint32_t baseaddr) :
    GenericBoard<CAENETControllerV288Register,cvA24_U_DATA>(bhandle, baseaddr)
  {
    //Reset();
  }

  CAENETControllerV288::~CAENETControllerV288()
  {;}

  void
  CAENETControllerV288::Reset() const
  {
    try {
      WriteRegister(kV288ModuleReset, (uint16_t)0x1);
      if (GetStatus().GetOperationStatus()!=CAENETControllerV288Status::Valid)
        throw Exception(__PRETTY_FUNCTION__, "Wrong status retrieved", JustWarning);
    } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to reset the CAENET/VME interface module", JustWarning);
    }
    usleep(5000);
  }

  void
  CAENETControllerV288::SendBuffer() const
  {
    uint16_t word = MSTIDENT;
    try {
      WriteRegister(kV288Transmission, word);
    } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to send buffer through the CAENET interface", JustWarning);
    }
  }

  std::vector<uint16_t>
  CAENETControllerV288::FetchBuffer(unsigned int num_words=1) const
  {
    unsigned short resp;
    if (!WaitForResponse(&resp) or resp!=0) {
      throw Exception(__PRETTY_FUNCTION__, "Wrong response retrieved", JustWarning);
    }
    std::vector<uint16_t> out;
    for (unsigned int i=0; i<num_words; i++) {
      uint16_t buf;
      *this >> buf;
      out.push_back(buf);
    }
    return out;
  }

  CAENETControllerV288Status
  CAENETControllerV288::GetStatus() const
  {
    uint16_t word = 0x0;
    try { ReadRegister(kV288Status, &word); } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve the status word", JustWarning);
    }
    return CAENETControllerV288Status(word);
  }

  bool
  CAENETControllerV288::WaitForResponse(uint16_t* response, unsigned int max_trials) const
  {
    uint16_t word = 0x0;
    unsigned int i = 0;
    do {
      ReadRegister(kV288DataBuffer, &word);
      if (GetStatus().GetOperationStatus()==CAENETControllerV288Status::Valid) {
        *response = word;
        return true;
      }
    } while (i<max_trials or max_trials<0);
    return false;
  }
}
