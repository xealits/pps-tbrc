#ifndef Message_h
#define Message_h

#include <string>
#include <iostream>

#include "MessageKeys.h"

/**
 * \brief Base message type
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 6 Apr 2015
 */
class Message
{
  public:
    inline Message() : fString("") {;}
    inline Message(const char* msg) : fString(msg) {;}
    inline Message(std::string msg) : fString(msg) {;}
    inline ~Message() {;}
    
    inline MessageKey GetKey() const { return INVALID_KEY; }
    inline std::string GetString() const { return fString; }
    
    inline bool IsFromWeb() const {
      size_t end;
      if ((end=fString.find("WebSocket"))!=std::string::npos) {
        return true;
      }
      return false;
    }
    
    inline void Dump(std::ostream& os=std::cout) const {
      os << "=============== General Message dump ==============" << std::endl
         << "  Value: " << fString << std::endl
         << "===================================================" << std::endl;      
    }

    
  protected:
    std::string fString;
};

#endif
