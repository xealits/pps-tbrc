#ifndef Message_h
#define Message_h

#include <string>
#include <iostream>

#include "MessageKeys.h"

/**
 * Base handler for messages to be transmitted through the socket
 * \brief Base socket message type
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 6 Apr 2015
 */
class Message
{
  public:
    /// Void message constructor
    inline Message() : fString("") {;}
    /// Construct a message from a string
    inline Message(const char* msg) : fString(msg) {;}
    /// Construct a message from a string
    inline Message(std::string msg) : fString(msg) {;}
    inline virtual ~Message() {;}
    
    /// Placeholder for the MessageKey retrieval method
    inline MessageKey GetKey() const { return INVALID_KEY; }
    /// Retrieve the string carried by this message as a whole
    inline std::string GetString() const { return fString; }
    
    /// Extract from any message its potential arrival from a WebSocket protocol
    inline bool IsFromWeb() const {
      if (fString.find("WebSocket")!=std::string::npos) return true;
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
