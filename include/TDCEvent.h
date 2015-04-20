#ifndef TDCEvent_h
#define TDCEvent_h

class TDCEvent
{
  public:
    typedef enum { Invalid=-1, GroupHeader=0, GroupTrailer, TDCHeader, TDCTrailer, LeadingEdge, TrailingEdge, Error, Debug } EventType;
  
  public:
    TDCEvent(const uint32_t& word) : fWord(word) {;}
    inline virtual ~TDCEvent() {;}
    
    /// Type of packet read out from the TDC
    inline EventType GetType() const {
      EventType type = static_cast<EventType>((fWord>>28)&0xF);
      if (type<GroupHeader or type>Debug) type = Invalid;
      return type;
    }
    /// Programmed identifier of master TDC
    inline unsigned int GetTDCId() const { return static_cast<unsigned int>((fWord>>24)&0xF); }
    /// Event identifier from event counter
    inline uint16_t GetEventId() const {
      if (GetType()<GroupHeader or GetType()>TDCTrailer) return -1;
      return static_cast<uint16_t>((fWord>>12)&0xFFF);
    }
    /// Total number of words in event (including headers and trailers)
    inline uint16_t GetWordCount() const {
      if (GetType()!=GroupTrailer or GetType()!=TDCTrailer) return -1;
      return static_cast<uint16_t>(fWord&0xFFF);
    }
    /// Bunch identifier of trigger (or trigger time tag)
    inline uint16_t GetBunchId() const {
      if (GetType()!=GroupHeader or GetType()!=TDCHeader) return -1;
      return static_cast<uint16_t>(fWord&0xFFF);
    }
    /// Leading edge measurement in programmed time resolution
    inline uint32_t GetLeadingTime(bool pair=false) const {
      if (GetType()!=LeadingEdge) return -1;
      if (pair) return static_cast<uint32_t>(fWord&0xFFF);
      else return static_cast<uint32_t>(fWord&0x7FFFF);
    }
    /// Width of pulse in programmed time resolution
    inline uint8_t GetWidth() const {
      if (GetType()!=LeadingEdge) return -1;
      return static_cast<uint8_t>((fWord>>12)&0x7F);
    }
    /// Trailing edge measurement in programmed time resolution
    inline uint32_t GetTrailingTime() const {
      if (GetType()!=TrailingEdge) return -1;
      return static_cast<uint32_t>(fWord&0x7FFFF);
    }
    /// Return error flags if an error condition has been detected
    inline uint16_t GetErrorFlags() const {
      if (GetType()!=Error) return -1;
      return static_cast<uint16_t>(fWord&0x7FFF);
    }
    
  private:
    uint32_t fWord;
};

#endif
