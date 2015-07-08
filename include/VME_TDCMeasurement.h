#ifndef VME_TDCMeasurement_h
#define VME_TDCMeasurement_h

#include <vector>
#include <map>

#include "VME_TDCEvent.h"

namespace VME
{
  class TDCMeasurement
  {
    public:
      inline TDCMeasurement() {;}
      inline TDCMeasurement(const std::vector<TDCEvent>& v) { SetEventsCollection(v); }
      inline ~TDCMeasurement() { fMap.clear(); }

      inline void Dump() {
        std::ostringstream os;
        /*for (std::map<Type,TDCEvent>::const_iterator e=fMap.begin(); e!=fMap.end(); e++) {
          os << "=> Type=" << e->first << std::endl;
        }*/
        os << "TDC/Channel Id: " << GetTDCId() << " / " << GetChannelId() << "\n\t"
           << "Event/bunch Id: " << GetEventId() << " / " << GetBunchId() << "\n\t";
        for (unsigned int i=0; i<NumEvents(); i++) {
          os << "----- Event " << i << " -----" << "\n\t"
             << "Leading time:   " << GetLeadingTime() << "\n\t"
             << "Trailing time:  " << GetTrailingTime() << "\n\t";
        }
        os << NumEvents() << " hits recorded";
        PrintInfo(os.str());
      }

      inline void SetEventsCollection(const std::vector<TDCEvent>& v) {
        unsigned int num_measurements = 0;
        fMap.clear(); fEvents.clear();
        TDCEvent leading; bool has_leading = false;
        for (std::vector<TDCEvent>::const_iterator e=v.begin(); e!=v.end(); e++) {
          switch (e->GetType()) {
            case TDCEvent::GlobalHeader:  fMap.insert(std::pair<TDCEvent::EventType,TDCEvent>(TDCEvent::GlobalHeader, *e)); break;
            case TDCEvent::GlobalTrailer: fMap.insert(std::pair<TDCEvent::EventType,TDCEvent>(TDCEvent::GlobalTrailer, *e)); break;
            case TDCEvent::TDCHeader:     fMap.insert(std::pair<TDCEvent::EventType,TDCEvent>(TDCEvent::TDCHeader, *e)); break;
            case TDCEvent::TDCTrailer:    fMap.insert(std::pair<TDCEvent::EventType,TDCEvent>(TDCEvent::TDCTrailer, *e)); break;
            case TDCEvent::ETTT:          fMap.insert(std::pair<TDCEvent::EventType,TDCEvent>(TDCEvent::ETTT, *e)); break;
            case TDCEvent::TDCMeasurement:
              if (!e->IsTrailing()) { leading = *e; has_leading = true; }
              else {
                if (!has_leading) return;
                fEvents.push_back(std::pair<TDCEvent,TDCEvent>(leading, *e));
                num_measurements++;
              }
              break;
            case TDCEvent::TDCError:
            case TDCEvent::Filler:
            default:
              break;
          }
        }
      }

      inline uint32_t GetLeadingTime(unsigned short event_id=0) {
        if (event_id>=fEvents.size()) { return 0; }
        return fEvents[event_id].first.GetLeadingTime();
      }
      inline uint32_t GetTrailingTime(unsigned short event_id=0) {
        if (event_id>=fEvents.size()) { return 0; }
        return fEvents[event_id].second.GetTrailingTime();
      }
      inline uint16_t GetToT(unsigned short event_id=0) {
        if (event_id>=fEvents.size()) { return 0; }
        uint32_t tt = GetTrailingTime(event_id), lt = GetLeadingTime(event_id);
        if ((tt-lt)>(1<<21)) tt += ((1<<21));
        return tt-lt;
      }
      inline uint16_t GetChannelId(unsigned short event_id=0) {
        if (event_id>=fEvents.size()) { return 0; }
        return fEvents[event_id].second.GetChannelId();
      }
      inline uint16_t GetTDCId() {
        if (!fMap.count(TDCEvent::TDCHeader)) { return 0; }
        return fMap[TDCEvent::TDCHeader].GetTDCId();
      }
      inline uint16_t GetEventId() {
        if (!fMap.count(TDCEvent::TDCHeader)) { return 0; }
        return fMap[TDCEvent::TDCHeader].GetEventId();
      }
      inline uint16_t GetBunchId() {
        if (!fMap.count(TDCEvent::TDCHeader)) { return 0; }
        return fMap[TDCEvent::TDCHeader].GetBunchId();
      }
      inline size_t NumEvents() const { return fEvents.size(); }

    private:
      std::map<TDCEvent::EventType,TDCEvent> fMap;
      std::vector< std::pair<TDCEvent,TDCEvent> > fEvents;
  };
}

#endif
