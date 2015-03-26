#ifndef Message_h
#define Message_h

#include <vector>
#include <string>

typedef std::vector<std::string> MessageMap;

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
      fMessage = ToObject();
    }
    inline Message(const char* key, const char* value) { SetKeyValue(key, value); }
    inline Message(const char* key, const int value) { SetKeyValue(key, value); }
    inline Message(const char* key, const float value) { SetKeyValue(key, value); }
    inline Message(const char* key, const double value) { SetKeyValue(key, value); }
    inline Message(std::string msg_s) { fMessageS = msg_s; }
    inline Message(MessageMap msg_m) { fMessage = msg_m; }
    inline ~Message() {;}
    
    /// Send a string-valued message
    inline void SetKeyValue(const char* key, const char* value) {
      fMessage.clear();
      fMessage.push_back(std::string(key));
      fMessage.push_back(std::string(value));
      fMessageS = ToString();
    }
    /// Send an integer-valued message
    inline void SetKeyValue(const char* key, int int_value) {
      std::ostringstream ss; ss << int_value;
      SetKeyValue(key, ss.str().c_str());
    }
    /// Send an float-valued message
    inline void SetKeyValue(const char* key, float float_value) {
      std::ostringstream ss; ss << float_value;
      SetKeyValue(key, ss.str().c_str());
    }
    /// Send an double-valued message
    inline void SetKeyValue(const char* key, double double_value) {
      std::ostringstream ss; ss << double_value;
      SetKeyValue(key, ss.str().c_str());
    }

    inline std::string GetKey() const { return (fMessage.size()>0) ? fMessage.at(0) : ""; }
    inline std::string GetValue() const { return (fMessage.size()>1) ? fMessage.at(1) : ""; }
    
    inline MessageMap ToObject() const {
      MessageMap out;
      int start = 0, end = 0;
      while ((end=fMessageS.find(':', start))!=std::string::npos) {
        out.push_back(fMessageS.substr(start, end-start));
        start = end + 1;
      }
      out.push_back(fMessageS.substr(start));
      return out;
    }
    inline std::string ToString() const {
      std::string out("");
      int i = 0;
      for (MessageMap::const_iterator m=fMessage.begin(); m!=fMessage.end(); m++, i++) {
        if (i>0) out += ":";
        out += (*m);
      }
      return out;
    }
    
    inline void Dump() const {
      std::cout << "================== Message dump ===================" << std::endl
                << "  Key:   " << GetKey() << std::endl
                << "  Value: " << GetValue() << std::endl
                << "===================================================" << std::endl;      
    }
    
  private:
    std::string fMessageS;
    MessageMap fMessage;
};

#endif
