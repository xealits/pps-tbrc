#ifndef FPGAHandler_h
#define FPGAHandler_h

#include "Client.h"
#include "TDCConfiguration.h"

#include <fstream>
#include <usb.h>

struct file_header_t {
  uint32_t magic;
  uint32_t run_id;
  uint32_t spill_id;
  TDCConfiguration config;
};

/**
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 14 Apr 2015
 */
class FPGAHandler : public Client
{
  public:
    FPGAHandler(int port, const char* dev);
    virtual ~FPGAHandler();
    
    void OpenFile();
    inline std::string GetFilename() const { return fFilename; }
    
    inline void SetConfiguration(const TDCConfiguration& c) { fConfig=c; }
    inline TDCConfiguration GetConfiguration() const { return fConfig; }
    void SendConfiguration();
    void ReadConfiguration();
    
    void WriteUSB(uint32_t word, uint8_t size) const;
    uint32_t FetchUSB(uint8_t size) const;
    
    void ReadBuffer();
    inline SocketType GetType() const { return DETECTOR; }

  private:
    std::string fDevice;
    std::string fFilename;
    std::ofstream fOutput;
    TDCConfiguration fConfig;
    bool fIsFileOpen;
};

#endif
