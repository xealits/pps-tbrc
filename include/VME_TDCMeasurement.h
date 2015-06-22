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
        LeadingEdge = 0x2,
        TrailingEdge = 0x3,
        ETTT = 0x4
      };

      inline TDCMeasurement() {;}
      inline TDCMeasurement(const std::vector<TDCEvent>& v) { SetEventsCollection(v); }
      inline ~TDCMeasurement() { fMap.clear(); }

      inline void Dump() {
        for (std::map<Type,TDCEvent>::const_iterator e=fMap.begin(); e!=fMap.end(); e++) {
          std::cout << "=> Type=" << e->first << std::endl;
        }
        std::cout << "--> TDC measurement" << std::endl
                  << "  Leading time:  " << GetLeadingTime() << std::endl
                  << "  Trailing time: " << GetTrailingTime() << std::endl;
      }

      inline void SetEventsCollection(const std::vector<TDCEvent>& v) {
        fMap.clear();
        for (std::vector<TDCEvent>::const_iterator e=v.begin(); e!=v.end(); e++) {
          switch (e->GetType()) {
            case TDCEvent::GlobalHeader:  fMap.insert(std::pair<Type,TDCEvent>(GlobalHeader, *e)); break;
            case TDCEvent::GlobalTrailer: fMap.insert(std::pair<Type,TDCEvent>(GlobalTrailer, *e)); break;
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

    private:
      std::map<Type,TDCEvent> fMap;
  };
}

#endif
