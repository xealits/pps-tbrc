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
      enum Type {
        GlobalHeader = 0x0,
        GlobalTrailer = 0x1,
        TDCHeader = 0x2,
        TDCTrailer = 0x3,
        LeadingEdge = 0x4,
        TrailingEdge = 0x5,
        ETTT = 0x6
      };

      inline TDCMeasurement() {;}
      inline TDCMeasurement(const std::vector<TDCEvent>& v) { SetEventsCollection(v); }
      inline ~TDCMeasurement() { fMap.clear(); }

      inline void Dump() {
        std::ostringstream os;
        /*for (std::map<Type,TDCEvent>::const_iterator e=fMap.begin(); e!=fMap.end(); e++) {
          os << "=> Type=" << e->first << std::endl;
        }*/
        os << "TDC/Channel Id: " << GetTDCId() << " / " << GetChannelId() << "\n\t"
           << "Event/bunch Id: " << GetEventId() << " / " << GetBunchId() << "\n\t"
           << "Leading time:   " << GetLeadingTime() << "\n\t"
           << "Trailing time:  " << GetTrailingTime();
        PrintInfo(os.str());
      }

      inline void SetEventsCollection(const std::vector<TDCEvent>& v) {
        fMap.clear();
        for (std::vector<TDCEvent>::const_iterator e=v.begin(); e!=v.end(); e++) {
          switch (e->GetType()) {
            case TDCEvent::GlobalHeader:  fMap.insert(std::pair<Type,TDCEvent>(GlobalHeader, *e)); break;
            case TDCEvent::GlobalTrailer: fMap.insert(std::pair<Type,TDCEvent>(GlobalTrailer, *e)); break;
            case TDCEvent::TDCHeader:     fMap.insert(std::pair<Type,TDCEvent>(TDCHeader, *e)); break;
            case TDCEvent::TDCTrailer:    fMap.insert(std::pair<Type,TDCEvent>(TDCTrailer, *e)); break;
            case TDCEvent::TDCMeasurement:
              if (e->IsTrailing()) fMap.insert(std::pair<Type,TDCEvent>(TrailingEdge, *e));
              else                 fMap.insert(std::pair<Type,TDCEvent>(LeadingEdge, *e));
              break;
            default:
              break;
          }
        }
      }

      inline uint32_t GetLeadingTime() {
        if (!fMap.count(LeadingEdge)) { return 0; }
        return fMap[LeadingEdge].GetLeadingTime();
      }
      inline uint32_t GetTrailingTime() {
        if (!fMap.count(TrailingEdge)) { return 0; }
        return fMap[TrailingEdge].GetTrailingTime();
      }
      inline uint16_t GetChannelId() {
        if (!fMap.count(LeadingEdge) or !fMap.count(TrailingEdge)) { return 0; }
        return fMap[TrailingEdge].GetChannelId();
      }
      inline uint16_t GetTDCId() {
        if (!fMap.count(TDCHeader)) { return 0; }
        return fMap[TDCHeader].GetTDCId();
      }
      inline uint16_t GetEventId() {
        if (!fMap.count(TDCHeader)) { return 0; }
        return fMap[TDCHeader].GetEventId();
      }
      inline uint16_t GetBunchId() {
        if (!fMap.count(TDCHeader)) { return 0; }
        return fMap[TDCHeader].GetBunchId();
      }

    private:
      std::map<Type,TDCEvent> fMap;
  };
}

#endif
