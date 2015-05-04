#ifndef TDCEvent_h
#define TDCEvent_h

#include <vector>

namespace VME
{
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
      TDCEvent() : fWord(0) {;}
      TDCEvent(const uint32_t& word) : fWord(word) {;}
      inline virtual ~TDCEvent() {;}
      
      inline void SetWord(const uint32_t& word) { fWord = word; }
      
      /// Type of packet read out from the TDC
      inline EventType GetType() const {
        return static_cast<EventType>((fWord>>27)&0x1F);
      }
      /// Programmed identifier of master TDC providing the event
      inline uint8_t GetTDCId() const {
        if (GetType()!=TDCHeader and GetType()!=TDCTrailer and GetType()!=TDCError) return -1;
        return static_cast<uint8_t>((fWord>>24)&0x3);
      }
      /// Event identifier from event counter
      inline uint16_t GetEventId() const {
        if (GetType()!=TDCHeader and GetType()!=TDCTrailer) return -1;
        return static_cast<uint16_t>((fWord>>12)&0xFFF);
      }
      /// Total number of words in event (including headers and trailers)
      inline uint16_t GetWordCount() const {
        if (GetType()!=TDCTrailer) return -1;
        return static_cast<uint16_t>(fWord&0xFFF);
      }
      inline uint8_t GetGeo() const {
        if (GetType()!=TDCTrailer) return -1;
        return static_cast<uint8_t>(fWord&0x1F);
      }
      inline uint8_t GetChannelId() const {
        if (GetType()!=TDCMeasurement) return -1;
        return static_cast<uint8_t>((fWord>>19)&0x7F);
      }
      /// Total number of events
      inline uint32_t GetEventCount() const {
        if (GetType()!=TDCTrailer) return -1;
        return static_cast<uint32_t>((fWord>>5)&0x3FFFF);
      }
      /// Bunch identifier of trigger (or trigger time tag)
      inline uint16_t GetBunchId() const {
        if (GetType()!=TDCHeader) return -1;
        return static_cast<uint16_t>(fWord&0xFFF);
      }
      /// Are we dealing with a trailing or a leading measurement?
      inline bool IsTrailing() const {
        if (GetType()!=TDCMeasurement) return -1;
        return static_cast<bool>((fWord>>26)&0x1);
      }
      /// Extended trigger time tag
      inline uint32_t GetETTT() const {
        if (GetType()!=ETTT) return -1;
        return static_cast<uint32_t>(fWord&0x3FFFFFF);
      }
      /**
       * \brief Leading edge measurement in programmed time resolution
       * \param[in] pair Are we dealing with a pair measurement?
       */
      inline uint32_t GetLeadingTime(bool pair=false) const {
        if (GetType()!=TDCMeasurement or IsTrailing()) return -1;
        if (pair) return static_cast<uint32_t>(fWord&0xFFF);
        else return static_cast<uint32_t>(fWord&0x7FFFF);
      }
      /// Width of pulse in programmed time resolution
      inline uint8_t GetWidth() const {
        if (GetType()!=TDCMeasurement) return -1;
        return static_cast<uint8_t>((fWord>>12)&0x7F);
      }
      /// Trailing edge measurement in programmed time resolution
      inline uint32_t GetTrailingTime() const {
        if (GetType()!=TDCMeasurement or !IsTrailing()) return -1;
        return static_cast<uint32_t>(fWord&0x7FFFF);
      }
      inline uint8_t GetStatus() const {
        if (GetType()!=GlobalTrailer) return -1;
        return static_cast<uint8_t>((fWord>>24)&0x7);
      }
      /// Return error flags if an error condition has been detected
      inline uint16_t GetErrorFlags() const {
        if (GetType()!=TDCError) return -1;
        return static_cast<uint16_t>(fWord&0x7FFF);
      }
      
    private:
      uint32_t fWord;
  };
  typedef std::vector<TDCEvent> TDCEventCollection;
}

#endif
