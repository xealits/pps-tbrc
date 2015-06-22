#ifndef TDCEvent_h
#define TDCEvent_h

#include <vector>

#include "Exception.h"

namespace VME
{
  /**
   * \brief TDC acquisition mode
   * \author Laurent Forthomme <laurent.forthomme@cern.ch>
   * \ingroup HPTDC
   */
  typedef enum {
    CONT_STORAGE,
    TRIG_MATCH,
  } ReadoutMode;
 
  /**
   * \brief Error flags handler
   * \author Laurent Forthomme <laurent.forthomme@cern.ch>
   * \date 22 Jun 2015
   * \ingroup HPTDC
   */
  class TDCErrorFlag {
    public:
      inline TDCErrorFlag(uint16_t ef) : fWord(ef) {;}
      //inline TDCErrorFlag(const TDCEvent& ev) : fWord(ev.GetWord()) {;}
      inline virtual ~TDCErrorFlag() {;}

      inline uint16_t GetWord() const { return fWord; }

      inline friend std::ostream& operator<<(std::ostream& os, const TDCErrorFlag& ef) {
        os << "Raw error word: ";
        for (unsigned int i=0; i<15; i++) {
          if (i%5==0) os << " ";
          os << ((ef.fWord>>i)&0x1);
        }
        os << "\n\t";
        for (unsigned int i=0; i<4; i++) {
           os << "===== Hit error in group " << i << " ========================== " << ef.HasGroupError(i) << "\n\t"
              << "  Read-out FIFO overflow:                             " << ef.HasReadoutFIFOOverflow(i) << "\n\t"
              << "  L1 buffer overflow:                                 " << ef.HasL1BufferOverflow(i) << "\n\t";
        }
        os << "Hits rejected because of programmed event size limit: " << ef.HasReachedEventSizeLimit() << "\n\t"
           << "Event lost (trigger FIFO overflow):                   " << ef.HasTriggerFIFOOverflow() << "\n\t"
           << "Internal fatal chip error has been detected:          " << ef.HasInternalChipError();
        return os;
      }
      inline void Dump() const {
        std::ostringstream os; os << this;
        PrintInfo(os.str());
      }
      /**
       * \brief Check whether hits have been lost from read-out FIFO overflow in a given group
       */
      inline bool HasReadoutFIFOOverflow(unsigned int group_id) const {
        return static_cast<bool>((fWord>>(3*group_id))&0x1);
      }
      /**
       * \brief Check whether hits have been lost from L1 buffer overflow in a given group
       */
      inline bool HasL1BufferOverflow(unsigned int group_id) const {
        return static_cast<bool>((fWord>>(1+3*group_id))&0x1);
      }
      /**
       * \brief Check whether hits have been lost due to error in a given group
       */
      inline bool HasGroupError(unsigned int group_id) const {
        return static_cast<bool>((fWord>>(2+3*group_id))&0x1);
      }
      /**
       * \brief Hits rejected because of programmed event size limit
       */
      inline bool HasReachedEventSizeLimit() const { return static_cast<bool>((fWord>>12)&0x1); }
      /**
       * \brief Event lost (trigger FIFO overflow)
       */
      inline bool HasTriggerFIFOOverflow() const { return static_cast<bool>((fWord>>13)&0x1); }
      /**
       * \brief Internal fatal chip error has been detected
       */
      inline bool HasInternalChipError() const { return static_cast<bool>((fWord>>14)&0x1); }
    private:
      uint16_t fWord;
  };

  /**
   * Object enabling to decipher any measurement/error/debug event returned by the
   * HPTDC chip
   * \brief HPTDC event parser
   * \author Laurent Forthomme <laurent.forthomme@cern.ch>
   * \date 4 May 2015
   * \ingroup HPTDC
   */
  class TDCEvent
  {
    public:
      enum EventType {
        TDCMeasurement = 0x0,
        TDCHeader = 0x1,
        TDCTrailer = 0x3,
        TDCError = 0x4,
        GlobalHeader = 0x8,
        GlobalTrailer = 0x10,
        ETTT = 0x11,
        Filler = 0x18
      };
    
    public:
      inline TDCEvent() : fWord(0) {;}
      inline TDCEvent(const TDCEvent& ev) : fWord(ev.fWord) {;}
      inline TDCEvent(const uint32_t& word) : fWord(word) {;}
      inline virtual ~TDCEvent() {;}

      inline void Dump() const {
        std::stringstream ss;
        ss << "Event dump\n\t"
           << "Type: 0x" << std::hex << GetType() << std::dec << "\n\t"
           << "Word:\n\t";
        for (int i=31; i>=0; i--) {
          ss << (unsigned int)((fWord>>i)&0x1);
          if (i%4==0) ss << " ";
        }
        PrintInfo(ss.str());
      }
      
      inline void SetWord(const uint32_t& word) { fWord = word; }
      inline uint32_t GetWord() const { return fWord; }
      
      /// Type of packet read out from the TDC
      inline EventType GetType() const {
        return static_cast<EventType>((fWord>>27)&0x1F);
      }
      /// Programmed identifier of master TDC providing the event
      inline unsigned int GetTDCId() const {
        if (GetType()!=TDCHeader and GetType()!=TDCTrailer and GetType()!=TDCError) return 0;
        return static_cast<unsigned int>((fWord>>24)&0x3);
      }
      /// Event identifier from event counter
      inline uint16_t GetEventId() const {
        if (GetType()!=TDCHeader and GetType()!=TDCTrailer) return 0;
        return static_cast<uint16_t>((fWord>>12)&0xFFF);
      }
      /// Total number of words in event (including headers and trailers)
      inline uint16_t GetWordCount() const {
        if (GetType()!=TDCTrailer) return 0;
        return static_cast<uint16_t>(fWord&0xFFF);
      }
      inline unsigned int GetGeo() const {
        if (GetType()!=TDCTrailer) return 0;
        return static_cast<unsigned int>(fWord&0x1F);
      }
      inline unsigned int GetChannelId() const {
        if (GetType()!=TDCMeasurement) return 0;
        //return static_cast<unsigned int>((fWord>>19)&0x7F);
        return static_cast<unsigned int>((fWord>>21)&0x1F);
      }
      /// Total number of events
      inline uint32_t GetEventCount() const {
        if (GetType()!=TDCTrailer) return 0;
        return static_cast<uint32_t>((fWord>>5)&0x3FFFF);
      }
      /// Bunch identifier of trigger (or trigger time tag)
      inline uint16_t GetBunchId() const {
        if (GetType()!=TDCHeader) return 0;
        return static_cast<uint16_t>(fWord&0xFFF);
      }
      /// Are we dealing with a trailing or a leading measurement?
      inline bool IsTrailing() const {
        if (GetType()!=TDCMeasurement) return 0;
        return static_cast<bool>((fWord>>26)&0x1);
      }
      /// Extended trigger time tag
      inline uint32_t GetETTT() const {
        if (GetType()!=ETTT) return 0;
        return static_cast<uint32_t>(fWord&0x3FFFFFF);
      }
      /**
       * \brief Leading edge measurement in programmed time resolution
       * \param[in] pair Are we dealing with a pair measurement?
       */
      inline uint32_t GetLeadingTime(bool pair=false) const {
        if (GetType()!=TDCMeasurement or IsTrailing()) return 0;
        if (pair) return static_cast<uint32_t>(fWord&0xFFF);
        else return static_cast<uint32_t>(fWord&0x7FFFF);
      }
      /// Width of pulse in programmed time resolution
      inline unsigned int GetWidth() const {
        if (GetType()!=TDCMeasurement) return 0;
        return static_cast<unsigned int>((fWord>>12)&0x7F);
      }
      /// Trailing edge measurement in programmed time resolution
      inline uint32_t GetTrailingTime() const {
        if (GetType()!=TDCMeasurement or !IsTrailing()) return 0;
        return static_cast<uint32_t>(fWord&0x7FFFF);
      }
      inline unsigned int GetStatus() const {
        if (GetType()!=GlobalTrailer) return 0;
        return static_cast<unsigned int>((fWord>>24)&0x7);
      }
      /// Return error flags if an error condition has been detected
      inline TDCErrorFlag GetErrorFlags() const {
        if (GetType()!=TDCError) return TDCErrorFlag(0);
        return TDCErrorFlag(fWord&0x7FFF);
      }
      
    private:
      uint32_t fWord;
  };
  typedef std::vector<TDCEvent> TDCEventCollection;
}

#endif
