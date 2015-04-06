#ifndef SocketMessage_h
#define SocketMessage_h

#include <utility>
#include <map>
#include <string>

#include "Message.h"

#include <iostream>

typedef std::pair<MessageKey, std::string> MessageMap;

/**
 * \brief Socket-passed message type
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 26 Mar 2015
 */
class SocketMessage : public Message
{
  public:
    inline SocketMessage() : Message("") {;}
    inline SocketMessage(Message msg) : Message(msg) {
      fMessage = Object();
    }
    inline SocketMessage(const char* msg_s) : Message(msg_s) { 
      fMessage = Object();
    }
    inline SocketMessage(MessageKey key, const char* value) : Message("") { SetKeyValue(key, value); }
    inline SocketMessage(MessageKey key, std::string value) : Message("") { SetKeyValue(key, value.c_str()); }
    inline SocketMessage(MessageKey key, const int value) : Message("") { SetKeyValue(key, value); }
    inline SocketMessage(MessageKey key, const float value) : Message("") { SetKeyValue(key, value); }
    inline SocketMessage(MessageKey key, const double value) : Message("") { SetKeyValue(key, value); }
    inline SocketMessage(std::string msg_s) : Message(msg_s) {
      fMessage = Object();
    }
    inline SocketMessage(MessageMap msg_m) : Message("") { fMessage = msg_m; }
    inline ~SocketMessage() {;}
    
    /// Send a string-valued message
    inline void SetKeyValue(MessageKey key, std::string value) {
      fMessage = make_pair(key, value);
      fString = String();
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

    inline std::string GetString() const { return fString; }
    inline MessageKey GetKey() const { return fMessage.first; }
    inline std::string GetValue() const { return fMessage.second; }
    inline int GetIntValue() const { return atoi(fMessage.second.c_str()); }
    
    inline void Dump(std::ostream& os=std::cout) const {
      os << "=============== Socket Message dump ===============" << std::endl
         << "  Key:   " << MessageKeyToString(GetKey()) << " (" << GetKey() << ")" << std::endl
         << "  Value: " << GetValue() << std::endl
         << "===================================================" << std::endl;      
    }
    
  private:
    inline MessageMap Object() const {
      MessageMap out;
      MessageKey key;
      std::string value;
      size_t end;
      if ((end=fString.find("WebSocket"))!=std::string::npos) {
        throw Exception(__PRETTY_FUNCTION__, "Trying to build a HTTPMessage", JustWarning);
        //return out;
      }
      if ((end=fString.find(':'))==std::string::npos) {
        std::ostringstream s;
        s << "Invalid message built! (\"" << fString << "\")";
        if (fString.empty()) 
          throw Exception(__PRETTY_FUNCTION__, s.str().c_str(), Fatal);
        throw Exception(__PRETTY_FUNCTION__, s.str().c_str(), JustWarning);
      }
      key = MessageKeyToObject(fString.substr(0, end).c_str());
      value = fString.substr(end+1);
      return make_pair(key, value);
    }
    inline std::string String() const {
      std::string out = MessageKeyToString(fMessage.first);
      out += ':';
      out += fMessage.second;
      //out += '\0';
      return out;
    }
  
    MessageMap fMessage;
};

#endif
