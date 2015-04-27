#ifndef TDCControl_h
#define TDCControl_h

#include "TDCRegister.h"
#include "TDCConstants.h"

/**
 * Object handling the control word provided by/to the HPTDC chip
 * \brief Control word to be sent to the HPTDC chip
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 24 Apr 2015
 * \ingroup HPTDC
 */
class TDCControl : public TDCRegister
{
  public:
    typedef enum {  } EnablePattern;
    typedef enum { R_EnablePattern, R_GlobalReset, R_DLLReset, R_PLLReset } RegisterName;
  
  public:
    inline TDCControl() : TDCRegister(TDC_CONTROL_BITS_NUM) {
      SetConstantValues();
    }
    inline TDCControl(const TDCControl& c) : TDCRegister(TDC_CONTROL_BITS_NUM, c) {
      SetConstantValues();
    }
    
    inline void SetEnablePattern(const EnablePattern& ep) {
      SetBits(kEnablePattern, static_cast<unsigned int>(ep), 4);
    }
    inline EnablePattern GetEnablePattern() const { 
      return static_cast<EnablePattern>(GetBits(kEnablePattern, 4));
    }
    inline void SetGlobalReset(const bool gr=true) { SetBits(kGlobalReset, gr, 1); }
    inline bool GetGlobalReset() const { return GetBits(kGlobalReset, 1); }
    
    inline void SetDLLReset(const bool dr=true) { SetBits(kDLLReset, dr, 1); }
    inline bool GetDLLReset() const { return GetBits(kDLLReset, 1); }
    inline void SetPLLReset(const bool pr=true) { SetBits(kPLLReset, pr, 1); }
    inline bool GetPLLReset() const { return GetBits(kPLLReset, 1); }
    
    inline void EnableChannel(unsigned int id) { SetBits(kEnableChannel+id, 1, 1); }
    inline void EnableAllChannels() {
      for (unsigned int i=0; i<TDC_NUM_CHANNELS; i++) { EnableChannel(i); }
    }
    inline void DisableChannel(unsigned int id) { SetBits(kEnableChannel+id, 0, 1); }
    inline void DisableAllChannels() {
      for (unsigned int i=0; i<TDC_NUM_CHANNELS; i++) { DisableChannel(i); }
    }
    
    inline void Dump(int verb=1, std::ostream& os=std::cout) const {
      os << "===================="
         << " TDC Control register dump "
         << "===================" << std::endl;
         if (verb>1) DumpRegister(os);
    }
    inline void SetConstantValues() {
      EnableAllChannels();
    }
    
  private:
    void SetControlParity(const bool cp=true) {
      SetBits(kControlParity, cp, 1);
    }
    
    static const bit kEnablePattern = 0;
    static const bit kGlobalReset = 4;
    static const bit kEnableChannel = 5;
    static const bit kDLLReset = 37;
    static const bit kPLLReset = 38;
    static const bit kControlParity = 39;
};

#endif
