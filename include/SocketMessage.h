#ifndef SocketMessage_h
#define SocketMessage_h

#include <utility>
#include <map>
#include <string>

#include "Message.h"

#include <iostream>

typedef std::pair<MessageKey, std::string> MessageMap;
typedef std::vector<std::string> VectorValue;

/**
 * \brief Socket-passed message type
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 26 Mar 2015
 * \ingroup Socket
 */
class SocketMessage : public Message
{
  public:
    inline SocketMessage() : Message("") {;}
    inline SocketMessage(const Message& msg) : Message(msg) {
      try { fMessage = Object(); } catch (Exception& e) { throw e; }
    }
    inline SocketMessage(const char* msg_s) : Message(msg_s) { 
      try { fMessage = Object(); } catch (Exception& e) { throw e; }
    }
    inline SocketMessage(std::string msg_s) : Message(msg_s) {
      try { fMessage = Object(); } catch (Exception& e) { throw e; }
    }
    /// Construct a socket message out of a key
    inline SocketMessage(const MessageKey& key) : Message() { SetKeyValue(key, ""); }
    /// Construct a socket message out of a key and a string-type value
    inline SocketMessage(const MessageKey& key, const char* value) : Message() { SetKeyValue(key, value); }
    /// Construct a socket message out of a key and a string-type value
    inline SocketMessage(const MessageKey& key, std::string value) : Message() { SetKeyValue(key, value.c_str()); }
    /// Construct a socket message out of a key and an integer-type value
    inline SocketMessage(const MessageKey& key, const int value) : Message() { SetKeyValue(key, value); }
    /// Construct a socket message out of a key and a float-type value
    inline SocketMessage(const MessageKey& key, const float value) : Message() { SetKeyValue(key, value); }
    /// Construct a socket message out of a key and a double precision-type value
    inline SocketMessage(const MessageKey& key, const double value) : Message() { SetKeyValue(key, value); }
    /// Construct a socket message out of a map of key/string-type value
    inline SocketMessage(MessageMap msg_m) : Message() { fMessage = msg_m; }
    inline ~SocketMessage() {;}
    
    /// String-valued message
    inline void SetKeyValue(const MessageKey& key, const char* value) {
      fMessage = make_pair(key, std::string(value));
      fString = String();
    }
    /// Send an integer-valued message
    inline void SetKeyValue(const MessageKey& key, int int_value) {
      std::ostringstream ss; ss << int_value;
      SetKeyValue(key, ss.str().c_str());
    }
    /// Float-valued message
    inline void SetKeyValue(const MessageKey& key, float float_value) {
      std::ostringstream ss; ss << float_value;
      SetKeyValue(key, ss.str().c_str());
    }
    /// Double-valued message
    inline void SetKeyValue(const MessageKey& key, double double_value) {
      std::ostringstream ss; ss << double_value;
      SetKeyValue(key, ss.str().c_str());
    }

    /// Extract the whole key:value message
    inline std::string GetString() const { return fString; }
    /// Extract the message's key
    inline MessageKey GetKey() const { return fMessage.first; }
    /// Extract the message's string value
    inline std::string GetValue() const { return fMessage.second; }
    /// Extract the message's string value (without the trailing endlines)
    inline std::string GetCleanedValue() const {
      std::string s = fMessage.second;
      s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
      s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
      return s;
    }
    /// Extract the message's integer value
    inline int GetIntValue() const { return atoi(fMessage.second.c_str()); }
    /// Extract the message's vector of string value
    inline VectorValue GetVectorValue() const {
      size_t start = 0, end = 0;
      VectorValue out;
      std::string value = GetValue();
      while ((end=value.find(';', start))!=std::string::npos) {
        out.push_back(value.substr(start, end-start));
        start = end + 1;
      }
      out.push_back(value.substr(start));
      return out;
    }
    
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
      if ((end=fString.find(':'))==std::string::npos) {
        std::ostringstream s; s << "Invalid message built! (\"" << fString << "\")";
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
      out += '\0';
      return out;
    }
  
    MessageMap fMessage;
};

#endif
