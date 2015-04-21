#ifndef TDCConfiguration_h
#define TDCConfiguration_h

#include <iostream>
#include <iomanip>
#include <stdint.h>

#define NUM_CHANNELS 32
#define WORD_SIZE 32
#define BITS_NUM 647

/**
 * Object handling the configuration word provided by/to the HPTDC chip
 * \brief Setup word to be sent to the HPTDC chip
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 16 Apr 2015
 */
class TDCConfiguration
{
  typedef uint16_t bit;
  typedef uint32_t word_t;
  
  public:  
    typedef enum {
      E_100ps=0, E_200ps, E_400ps, E_800ps, E_1p6ns, E_3p12ns, E_6p25ns, E_12p5ns
    } EdgeResolution;
    typedef enum { DT_5ns=0, DT_10ns, DT_30ns, DT_100ns } DeadTime;
    typedef enum {
      W_100ps=0, W_200ps, W_400ps, W_800ps, W_1p6ns, W_3p2ns, W_6p25ns,
      W_12p5ns, W_25ns, W_50ns, W_100ns, W_200ns, W_400ns, W_800ns
    } WidthResolution;
    typedef enum {
      VernierError=0x1, CoarseError=0x2, ChannelSelectError=0x4,
      L1BufferParityError=0x8, TriggerFIFOParityError=0x10,
      TriggerMatchingError=0x20, ReadoutFIFOParityError=0x40,
      ReadoutStateError=0x80, SetupParityError=0x100, ControlParityError=0x200,
      JTAGInstructionParityError=0x400
    } EnabledError;
  
  public:
    TDCConfiguration();
    //inline TDCConfiguration(const word_t* c) : fWord(c) {;}
    inline virtual ~TDCConfiguration() {;}
    
    /// Set one single word in the configuration
    inline void SetWord(const unsigned int i, const word_t word) {
      if (i<0 or i>=kNumWords) return;
      fWord[i] = word;
    }
    /// Retrieve one single word from the configuration
    inline word_t GetWord(const unsigned int i) const {
      if (i<0 or i>=kNumWords) return -1;
      return fWord[i];
    }
    /**
     * Return the number of words making up the full configuration word.
     * \brief Number of words in the configuration
     */
    inline uint8_t GetNumWords() const { return kNumWords; }
    
    inline void SetEnableError(const uint16_t& err) { SetBits(kEnableError, err, 11); }
    inline uint16_t GetEnableError() const { return static_cast<uint16_t>(GetBits(kEnableError, 11)); }
    inline void SetEdgeResolution(const EdgeResolution r) { SetBits(kLeadingResolution, r, 3); }
    inline EdgeResolution GetEdgeResolution() const { return static_cast<EdgeResolution>(GetBits(kLeadingResolution, 3)); }
    /**
     * Set the maximum number of hits that can be recorded for each event.
     * It is always rounded to the next power of 2 (in the range 0-128), and
     * if bigger than 128 then set to unimited.
     * \brief Set the maximum number of hits per event
     */
    inline void SetMaxEventSize(unsigned int sz) {
      uint8_t size;
      if (sz<0) return;
      else if (sz==0) size = 0x0; // no hit
      else if (sz>0) size = 0x1; // 1 hit
      else if (sz>1) size = 0x2; // 2 hits
      else if (sz>2) size = 0x3; // 4 hits
      else if (sz>4) size = 0x4; // 8 hits
      else if (sz>8) size = 0x5; // 16 hits
      else if (sz>16) size = 0x6; // 32 hits
      else if (sz>32) size = 0x7; // 64 hits
      else if (sz>64) size = 0x8; // 128 hits
      else if (sz>128) size = 0x9; // no limit
      SetBits(kMaxEventSize, size, 4);
    }
    /// Extract the maximum number of hits per event
    inline uint8_t GetMaxEventSize() const { 
      uint8_t max = static_cast<uint8_t>(GetBits(kMaxEventSize, 4));
      if (max==0) return 0;
      else if (max<0xA) return (1<<(max-1));
      else return -1;
    }
    /**
     * Set whether or not hits are rejected once FIFO is full.
     * \brief Reject hits when readout FIFO full.
     */
    inline void SetRejectFIFOFull(bool rej=true) { SetBits(kRejectFIFOFull, rej, 1); }
    /**
     * Extract whether or not hits are rejected once FIFO is full.
     * \brief Are hits rejected when readout FIFO is full?
     */
    inline bool GetRejectFIFOFull() const { return static_cast<bool>(GetBits(kRejectFIFOFull, 1)); }
    inline void SetChannelOffset(int channel, uint16_t offset) {
      if (channel>=NUM_CHANNELS or channel<0) return;
      SetBits(kOffset0-9*channel, offset, 9);
    }
    inline uint16_t GetChannelOffset(int channel) const {
      if (channel>=NUM_CHANNELS or channel<0) return -1;
      return static_cast<uint16_t>(GetBits(kOffset0-9*channel, 9));
    }
    inline void SetAllChannelsOffset(uint16_t offset) {
      for (int i=0; i<NUM_CHANNELS; i++) {
        SetChannelOffset(i, offset);
      }
    }
    /// Set the DLL taps adjustments with a resolution of ~10 ps
    inline void SetDLLAdjustment(int tap, uint8_t adj) {
      if (tap>=NUM_CHANNELS or tap<0) return;
      SetBits(kDLLTapAdjust0+3*tap, adj, 3);
    }
    inline uint8_t GetDLLAdjustment(int tap) const {
      if (tap>=NUM_CHANNELS or tap<0) return -1;
      return static_cast<uint8_t>(GetBits(kDLLTapAdjust0+3*tap, 3));
    }
    inline void SetAllTapsDLLAdjustment(uint8_t adj) {
      for (int i=0; i<NUM_CHANNELS; i++) {
        SetDLLAdjustment(i, adj);
      }
    }
    inline void SetRCAdjustment(int tap, uint8_t adj) {
      if (tap>3 or tap<0) return;
      SetBits(kRCAdjust0+3*tap, adj, 3);
    }
    inline uint8_t GetRCAdjustment(int tap) {
      if (tap>3 or tap<0) return -1;
      return static_cast<uint8_t>(GetBits(kRCAdjust0+3*tap, 3));
    }
    inline void SetWidthResolution(const WidthResolution r) { SetBits(kWidthSelect, r, 4); }
    inline WidthResolution GetWidthResolution() const { return static_cast<WidthResolution>(GetBits(kWidthSelect, 4)); }
    inline void SetDeadTime(const DeadTime dt) { SetBits(kDeadTime, dt, 2); }
    inline DeadTime GetDeadTime() const { return static_cast<DeadTime>(GetBits(kDeadTime, 2)); }
    /// Enable the detection of leading edges
    inline void SetLeadingMode(const bool lead=true) { SetBits(kLeading, lead, 1); }
    /// Extract the status for the detection of leading edges
    inline bool GetLeadingMode() const { return static_cast<bool>(GetBits(kLeading, 1)); }
    /// Enable/disable the detection of trailing edges
    inline void SetTrailingMode(const bool trail=true) { SetBits(kTrailing, trail, 1); }
    /// Extract the status for the detection of trailing edges
    inline bool GetTrailingMode() const { return static_cast<bool>(GetBits(kTrailing, 1)); }
    inline void SetTriggerMatchingMode(const bool trig=true) { SetBits(kEnableMatching, trig, 1); }
    inline bool GetTriggerMatchingMode() const { return static_cast<bool>(GetBits(kEnableMatching, 1)); }
    inline void SetEdgesPairing(const bool pair=true) { SetBits(kEnablePair, pair, 1); }
    inline bool GetEdgesPairing() const { return static_cast<bool>(GetBits(kEnablePair, 1)); }
    
    void Dump(int verb=1, std::ostream& os=std::cout) const;
    
  private:
    /**
     * Set a fixed amount of bits in the full configuration word
     * \brief Set bits in the configuration word
     * \param[in] lsb Least significant bit of the word to set
     * \param[in] word Word to set
     * \param[in] size Size of the word to set
     */
    void SetBits(uint16_t lsb, uint16_t word, uint8_t size);
    /**
     * Extract a fixed amount of bits from the full configuration word
     * \brief Extract bits from the configuration word
     * \param[in] lsb Least significant bit of the word to retrieve
     * \param[in] size Size of the word to retrieve
     */
    uint16_t GetBits(uint16_t lsb, uint8_t size) const;
    
    static const uint8_t kNumWords = BITS_NUM/WORD_SIZE+1;
    word_t fWord[kNumWords];
    
    // Least Significant Bits
    static const bit kEnableError = 6;
    static const bit kLeadingResolution = 84;
    static const bit kMaxEventSize = 116;
    static const bit kRejectFIFOFull = 120;
    static const bit kOffset0 = 438;
    static const bit kDLLTapAdjust0 = 459;
    static const bit kRCAdjust0 = 555;
    static const bit kWidthSelect = 571;
    static const bit kDeadTime = 584;
    static const bit kTrailing = 588;
    static const bit kLeading = 589;
    static const bit kEnableMatching = 639;
    static const bit kEnablePair = 640;
    
};

#endif
