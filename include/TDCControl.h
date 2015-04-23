#ifndef TDCControl_h
#define TDCControl_h

#include "TDCRegister.h"

#define CONTROL_BITS_NUM 40

class TDCControl : public TDCRegister
{
  public:
    typedef enum {
      
    } EnablePattern;
  
  public:
    inline TDCControl() : TDCRegister(CONTROL_BITS_NUM) {
      //SetConstantValues();
    }
    inline TDCControl(const TDCControl& c) : TDCRegister(CONTROL_BITS_NUM) {
      for (size_t i=0; i<GetNumWords(); i++) { fWord[i] = c.fWord[i]; }
    }
    inline virtual ~TDCControl() {
      delete fWord;
    }
    
    void SetEnablePattern(const EnablePattern& ep) {
      SetBits(kEnablePattern, static_cast<int>(ep), 4);
    }
    void SetGlobalReset(const bool gr=true) {
      SetBits(kGlobalReset, gr, 1);
    }
    void SetDLLReset(const bool dr=true) {
      SetBits(kDLLReset, dr, 1);
    }
    void SetPLLReset(const bool pr=true) {
      SetBits(kPLLReset, pr, 1);
    }
    void SetControlParity(const bool cp=true) {
      SetBits(kControlParity, cp, 1);
    }
    
    inline void Dump(int verb=1, std::ostream& os=std::cout) const {
      os << "===================="
         << " TDC Control register dump "
         << "===================" << std::endl;
         if (verb>1) DumpRegister(CONTROL_BITS_NUM, os);
    }
    
  private:
    static const bit kEnablePattern = 0;
    static const bit kGlobalReset = 4;
    static const bit kEnableChannel = 5;
    static const bit kDLLReset = 37;
    static const bit kPLLReset = 38;
    static const bit kControlParity = 39;
};

#endif
