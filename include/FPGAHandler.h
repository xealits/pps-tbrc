#ifndef FPGAHandler_h
#define FPGAHandler_h

#include "Listener.h"

#include <fstream>
#include <usb.h>

struct file_header_t {
  uint16_t magic;
  uint32_t run_id;
  uint32_t spill_id;
  //uint64_t configuration;
};

class FPGAConfiguration
{
  public:
    FPGAConfiguration();
    inline virtual ~FPGAConfiguration() {;}
    
    //FIXME FIXME FIXME burp...
    // Set...() ... Get...()
    
  private:
    uint32_t fWord;
};

class FPGAHandler : public Listener
{
  public:
    FPGAHandler(int port, const char* dev);
    virtual ~FPGAHandler();
    
    void OpenFile();
    inline std::string GetFilename() const { return fFilename; }
    
    void SendConfiguration(const FPGAConfiguration& c);
    FPGAConfiguration ReadConfiguration();
    
    void ReadBuffer();

  private:
    std::string fDevice;
    std::string fFilename;
    std::ofstream fOutput;
    bool fIsFileOpen;
};

#endif
