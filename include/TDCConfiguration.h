#ifndef TDCConfiguration_h
#define TDCConfiguration_h

#include <iostream>
#include <iomanip>
#include <stdint.h>

#define WORD_SIZE 32
#define BITS_NUM 647

/**
 * Object handling the configuration word provided by/to the HPTDC chip
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 16 Apr 2015
 */
class TDCConfiguration
{
  typedef uint16_t bit;
  typedef uint32_t word_t;
  
  public:  
    typedef enum { E_100PS=0, E_200PS, E_400PS, E_800PS, E_1600PS, E_3120PS, E_6250PS, E_12500PS } EdgeResolution;
    typedef enum { DT_5NS=0, DT_10NS, DT_30NS, DT_100NS } DeadTime;
    typedef enum { W_100PS=0, W_200PS, W_400PS, W_800PS, W_1p6NS, W_3p2NS, W_6p25NS, W_12p5NS, W_25NS, W_50NS, W_100NS, W_200NS, W_400NS, W_800NS } WidthResolution;
  
  public:
    TDCConfiguration();
    //inline TDCConfiguration(const word_t* c) : fWord(c) {;}
    inline virtual ~TDCConfiguration() {;}
    
    inline void SetEdgeResolution(const EdgeResolution r) { SetWord(kLeadingResolution, r, 3); }
    inline EdgeResolution GetEdgeResolution() const { return static_cast<EdgeResolution>(GetWord(kLeadingResolution, 3)); }
    
    void SetChannelOffset(int channel, uint16_t offset);
    uint16_t GetChannelOffset(int channel);
    inline void SetAllChannelsOffset(short offset) {
      for (int i=0; i<32; i++) SetChannelOffset(i, offset);
    }
    
    inline void SetWidthResolution(const WidthResolution r) { SetWord(kWidthSelect, r, 4); }
    inline WidthResolution GetWidthResolution() const { return static_cast<WidthResolution>(GetWord(kWidthSelect, 4)); }
    inline void SetDeadTime(const DeadTime dt) { SetWord(kDeadTime, dt, 2); }
    inline DeadTime GetDeadTime() const { return static_cast<DeadTime>(GetWord(kDeadTime, 2)); }
    /// Enable the detection of leading edges
    inline void SetLeadingMode(const bool lead=true) { SetWord(kLeading, lead, 1); }
    /// Extract the status for the detection of leading edges
    inline bool GetLeadingMode() const { return static_cast<bool>(GetWord(kLeading, 1)); }
    /// Enable/disable the detection of trailing edges
    inline void SetTrailingMode(const bool trail=true) { SetWord(kTrailing, trail, 1); }
    /// Extract the status for the detection of trailing edges
    inline bool GetTrailingMode() const { return static_cast<bool>(GetWord(kTrailing, 1)); }
    inline void SetTriggerMatchingMode(const bool trig=true) { SetWord(kEnableMatching, trig, 1); }
    inline bool GetTriggerMatchingMode() const { return static_cast<bool>(GetWord(kEnableMatching, 1)); }
    inline void SetEdgesPairing(const bool pair=true) { SetWord(kEnablePair, pair, 1); }
    inline bool GetEdgesPairing() const { return static_cast<bool>(GetWord(kEnablePair, 1)); }
    
    void Dump(std::ostream& os=std::cout) const;
    
  private:
    void SetWord(uint16_t lsb, uint16_t word, uint8_t size);
    uint16_t GetWord(uint16_t lsb, uint8_t size) const;
    
    static const uint8_t kNumWords = BITS_NUM/WORD_SIZE+1;
    word_t fWord[kNumWords];
    
    // Sizes
    static const uint16_t kSixteenBits = 0xFFFF;
    static const uint8_t kEightBits = 0xFF;
    static const char kOneBit = 0x1;
    
    // First bits
    static const bit kLeadingResolution = 84;
    static const bit kOffset0 = 438;
    static const bit kWidthSelect = 571;
    static const bit kDeadTime = 584;
    static const bit kTrailing = 588;
    static const bit kLeading = 589;
    static const bit kEnableMatching = 639;
    static const bit kEnablePair = 640;
    
};

#endif
