#ifndef Exception_h
#define Exception_h

#include <string>

/**
 * \brief Enumeration of exception severities
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 27 Mar 2015
 */
typedef enum
{
  Undefined,
  JustWarning,
  Fatal
} ExceptionType;

/**
 * \brief A simple exception handler
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 24 Mar 2015
 */
class Exception
{
  public:
    inline Exception(const char* from, const char* desc, ExceptionType type=Undefined, const int id=0) {
      fFrom = from;
      fDescription = desc;
      fType = type;
      fErrorNumber = id;
    }
    inline ~Exception() {
      if (Type()==Fatal) exit(0);
    }
    
    inline std::string From() const { return fFrom; }
    inline int ErrorNumber() const { return fErrorNumber; }
    inline std::string Description() const { return fDescription; }
    inline ExceptionType Type() const { return fType; }
    inline std::string TypeString() const {
      switch (Type()) {
        case JustWarning: return "\033[34;1mJustWarning\033[0m";
        case Fatal: return "\033[31;1mFatal\033[0m";
        case Undefined:
        default:
          return "\33[7;1mUndefined\033[0m";
      }
    }
    
    inline void Dump() const {
      std::cerr << "=============== Exception detected! ===============" << std::endl
                << "  Type:        " << TypeString() << std::endl
                << "  Raised by:   " << From() << std::endl;
      std::cerr << "  Description: " << std::endl
                << "    " << Description() << std::endl;
      if (ErrorNumber()!=0)
        std::cerr << "---------------------------------------------------" << std::endl
                  << "  Error #" << ErrorNumber() << std::endl;
      std::cerr << "===================================================" << std::endl;
    }
    
  private:
    std::string fFrom;
    std::string fDescription;
    ExceptionType fType;
    int fErrorNumber;
};

#endif

