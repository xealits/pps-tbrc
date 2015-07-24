#ifndef VME_CAENETControllerV288_h
#define VME_CAENETControllerV288_h

#include "VME_GenericBoard.h"
#include <vector>

namespace VME
{
  enum CAENETControllerV288Register {
    kV288DataBuffer   = 0x00, // R/W
    kV288Status       = 0x02, // R
    kV288Transmission = 0x04, // W
    kV288ModuleReset  = 0x06, // W
    kV288IRQVector    = 0x08  // W
  };

  class CAENETControllerV288Status
  {
    public:
      inline CAENETControllerV288Status(uint16_t word) : fWord(word) {;}
      inline ~CAENETControllerV288Status() {;}

      enum OperationStatus { Valid=0x0, Invalid=0x1 };
      inline OperationStatus GetOperationStatus() const { return static_cast<OperationStatus>(fWord&0x1); }

    private:
      uint16_t fWord;
  };

  /**
   * \brief Handler for a CAEN V288 CAENET controller
   * \author Laurent Forthomme <laurent.forthomme@cern.ch>
   * \date 23 Jul 2015
   */
  class CAENETControllerV288 : public GenericBoard<CAENETControllerV288Register,cvA24_U_DATA>
  {
    public:
      CAENETControllerV288(int32_t handle, uint32_t baseaddr);
      ~CAENETControllerV288();

      CAENETControllerV288Status GetStatus() const;

      /// Fill the buffer with an additional 16-bit word
      friend void operator<<(uint16_t& word, CAENETControllerV288& cnt) {
        try {
          cnt.WriteRegister(kV288DataBuffer, word);
          cnt.fNumWordsInBuffer++;
        } catch (Exception& e) {
          e.Dump();
          throw Exception(__PRETTY_FUNCTION__, "Failed to fill the buffer with an additional word", JustWarning);
        }
      }
      /// Read back a 16-bit word from the buffer
      friend uint16_t& operator>>(uint16_t& word, const CAENETControllerV288& cnt) {
        try { cnt.ReadRegister(kV288DataBuffer, &word); } catch (Exception& e) {
          e.Dump();
          throw Exception(__PRETTY_FUNCTION__, "Failed to retrieve an additional word from the buffer", JustWarning);
        }
        return word;
      }

      /// Send the whole buffer through the network
      void Send();
      /// Retrieve the network buffer
      std::vector<uint16_t> Receive() const;
      bool WaitForResponse(uint16_t* response, unsigned int max_trials=-1) const;

    private:
      unsigned int fNumWordsInBuffer;
  };
}

#endif
