#ifndef Exception_h
#define Exception_h

#include <string>

class Exception
{
  public:
    inline Exception(const char* from, const char* desc, const int id=0) {
      fFrom = from;
      fDescription = desc;
      fErrorNumber = id;
    }
    inline virtual ~Exception() {;}
    
    inline std::string From() const { return fFrom; }
    inline int ErrorNumber() const { return fErrorNumber; }
    inline std::string Description() const { return fDescription; }
    
    inline void Dump() const {
      std::cerr << "=============== Exception detected! ===============" << std::endl
                << "  Raised by:   " << From() << std::endl;
      if (ErrorNumber()!=0)
        std::cerr << "  Error #      " << ErrorNumber() << std::endl;
      std::cerr << "  Description: " << Description() << std::endl
                << "===================================================" << std::endl;
    }
    
  private:
    std::string fFrom;
    std::string fDescription;
    int fErrorNumber;
};

#endif

