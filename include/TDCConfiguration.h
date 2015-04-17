#ifndef TDCConfiguration_h
#define TDCConfiguration_h

/**
 * Object handling the configuration word provided by/to the HPTDC chip
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 16 Apr 2015
 */
class TDCConfiguration
{
  typedef unsigned short bit;  
  
  public:
    TDCConfiguration();
    inline virtual ~TDCConfiguration() {;}
    
    //FIXME FIXME FIXME burp...
    // Set...() ... Get...()
    void SetChannelOffset(int channel, short offset);
    short GetChannelOffset(int channel);
    inline void SetAllChannelsOffset(short offset) {
      for (int i=0; i<32; i++) SetChannelOffset(i, offset);
    }
    
    void Dump() const;
    
  private:
    long long fWord;
    static const bit kErrorMark = 4;
    static const bit kErrorBypass = 5;
    static const bit kDAQModeBit = 1;
    
    
};

#endif
