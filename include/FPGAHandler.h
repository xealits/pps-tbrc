#ifndef FPGAHandler_h
#define FPGAHandler_h

#include "Listener.h"
#include "TDCConfiguration.h"

#include <fstream>
#include <usb.h>

struct file_header_t {
  uint32_t magic;
  uint32_t run_id;
  uint32_t spill_id;
  //uint64_t configuration;
};

/**
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 14 Apr 2015
 */
class FPGAHandler : public Listener
{
  public:
    FPGAHandler(int port, const char* dev);
    virtual ~FPGAHandler();
    
    void OpenFile();
    inline std::string GetFilename() const { return fFilename; }
    
    void SendConfiguration(const TDCConfiguration& c);
    TDCConfiguration ReadConfiguration();
    
    void ReadBuffer();

  private:
    std::string fDevice;
    std::string fFilename;
    std::ofstream fOutput;
    bool fIsFileOpen;
};

#endif
