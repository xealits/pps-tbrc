#ifndef Message_h
#define Message_h

#include <utility>
#include <map>
#include <string>

#include "MessageKeys.h"

#include <iostream>

typedef std::pair<MessageKey, std::string> MessageMap;

/**
 * \brief Socket-passed message type
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 26 Mar 2015
 */
class Message
{
  public:
    inline Message() {;}
    inline Message(const char* msg_s) { 
      fMessageS = std::string(msg_s);
      fMessage = Object();
    }
    inline Message(MessageKey key, const char* value) { SetKeyValue(key, value); }
    inline Message(MessageKey key, std::string value) { SetKeyValue(key, value.c_str()); }
    inline Message(MessageKey key, const int value) { SetKeyValue(key, value); }
    inline Message(MessageKey key, const float value) { SetKeyValue(key, value); }
    inline Message(MessageKey key, const double value) { SetKeyValue(key, value); }
    inline Message(std::string msg_s) {
      fMessageS = msg_s;
      fMessage = Object();
    }
    inline Message(MessageMap msg_m) { fMessage = msg_m; }
    inline ~Message() {;}
    
    /// Send a string-valued message
    inline void SetKeyValue(MessageKey key, std::string value) {
      fMessage = make_pair(key, value);
      fMessageS = String();
    }
    inline void SetKeyValue(MessageKey key, const char* value) {
      SetKeyValue(key, std::string(value));
    }
    /// Send an integer-valued message
    inline void SetKeyValue(MessageKey key, int int_value) {
      std::ostringstream ss; ss << int_value;
      SetKeyValue(key, ss.str());
    }
    /// Send an float-valued message
    inline void SetKeyValue(MessageKey key, float float_value) {
      std::ostringstream ss; ss << float_value;
      SetKeyValue(key, ss.str());
    }
    /// Send an double-valued message
    inline void SetKeyValue(MessageKey key, double double_value) {
      std::ostringstream ss; ss << double_value;
      SetKeyValue(key, ss.str());
    }

    inline std::string GetString() const { return fMessageS; }
    inline MessageKey GetKey() const { return fMessage.first; }
    inline std::string GetValue() const { return fMessage.second; }
    inline int GetIntValue() const { return atoi(fMessage.second.c_str()); }
    
    inline void Dump(std::ostream& os=std::cout) const {
      os << "================== Message dump ===================" << std::endl
         << "  Key:   " << MessageKeyToString(GetKey()) << " (" << GetKey() << ")" << std::endl
         << "  Value: " << GetValue() << std::endl
         << "===================================================" << std::endl;      
    }
    
  private:
    inline MessageMap Object() const {
      MessageMap out;
      MessageKey key;
      std::string value;
      int end;
      if ((end=fMessageS.find(':'))==std::string::npos) {
        std::ostringstream s;
        s << "Invalid message built! (\"" << fMessageS << "\")";
        if (fMessageS.empty()) 
          throw Exception(__PRETTY_FUNCTION__, s.str().c_str(), Fatal);
        throw Exception(__PRETTY_FUNCTION__, s.str().c_str(), JustWarning);
      }
      key = MessageKeyToObject(fMessageS.substr(0, end).c_str());
      value = fMessageS.substr(end+1);
      return make_pair(key, value);
    }
    inline std::string String() const {
      std::string out = MessageKeyToString(fMessage.first);
      out += ':';
      out += fMessage.second;
      //out += '\0';
      return out;
    }
  
    std::string fMessageS;
    MessageMap fMessage;
};

#endif
