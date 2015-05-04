#ifndef FileReader_h
#define FileReader_h

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>

#include "FileConstants.h"
#include "Exception.h"

#include "TDCEvent.h"

class FileReader
{
  public:
    FileReader(std::string name);
    ~FileReader();
    
    inline unsigned int GetNumTDCs() const { return fHeader.num_hptdc; }
    
    inline TDCEvent GetNextEvent();
    
  private:
    std::ifstream fFile;
    file_header_t fHeader;
};

#endif
