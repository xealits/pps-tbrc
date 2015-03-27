#ifndef Message_h
#define Message_h

#include <utility>
#include <map>
#include <string>

#include "MessageKeys.h"

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
    inline Message(char* msg_s) { 
      fMessageS = std::string(msg_s);
      fMessage = Object();
    }
    inline Message(MessageKey key, const char* value) { SetKeyValue(key, value); }
    inline Message(MessageKey key, const int value) { SetKeyValue(key, value); }
    inline Message(MessageKey key, const float value) { SetKeyValue(key, value); }
    inline Message(MessageKey key, const double value) { SetKeyValue(key, value); }
    inline Message(std::string msg_s) { fMessageS = msg_s; }
    inline Message(MessageMap msg_m) { fMessage = msg_m; }
    inline ~Message() {;}
    
    /// Send a string-valued message
    inline void SetKeyValue(MessageKey key, const char* value) {
      fMessage = make_pair(key, std::string(value));
      fMessageS = String();
    }
    /// Send an integer-valued message
    inline void SetKeyValue(MessageKey key, int int_value) {
      std::ostringstream ss; ss << int_value;
      SetKeyValue(key, ss.str().c_str());
    }
    /// Send an float-valued message
    inline void SetKeyValue(MessageKey key, float float_value) {
      std::ostringstream ss; ss << float_value;
      SetKeyValue(key, ss.str().c_str());
    }
    /// Send an double-valued message
    inline void SetKeyValue(MessageKey key, double double_value) {
      std::ostringstream ss; ss << double_value;
      SetKeyValue(key, ss.str().c_str());
    }

    inline MessageKey GetKey() const { return fMessage.first; }
    inline std::string GetValue() const { return fMessage.second; }
    
    inline MessageMap Object() const {
      MessageMap out;
      MessageKey key;
      std::string value;
      int start = 0, end = 0, idx = 0;
      while ((end=fMessageS.find(':', start))!=std::string::npos) {
        if (idx==0) key = MessageKeyToObject(fMessageS.substr(start, end-start).c_str());
        else value = fMessageS.substr(start, end-start);
        start = end + 1;
        idx++;
      }
      return make_pair(key, value);
    }
    inline std::string String() const {
      std::string out = MessageKeyToString(fMessage.first);
      out += ':';
      out += fMessage.second;
      return out;
    }
    
    inline void Dump() const {
      std::cout << "================== Message dump ===================" << std::endl
                << "  Key:   " << MessageKeyToString(GetKey()) << " (" << GetKey() << ")" << std::endl
                << "  Value: " << GetValue() << std::endl
                << "===================================================" << std::endl;      
    }
    
  private:
    std::string fMessageS;
    MessageMap fMessage;
};

#endif
