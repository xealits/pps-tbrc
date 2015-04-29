#ifndef FileReader_h
#define FileReader_h

#include <string>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

#include "FileConstants.h"
#include "TDCSetup.h"
#include "Exception.h"

class FileReader
{
  public:
    FileReader(std::string name);
    ~FileReader();
    
    inline unsigned int GetNumTDCs() const { return fHeader.num_hptdc; }
    //inline TDCSetup GetTDCSetup() const { return fTDCSetup; }
    
  private:
    std::ifstream fFile;
    file_header_t fHeader;
    TDCSetup fTDCSetup;
};

#endif
