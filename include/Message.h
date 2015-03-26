#ifndef Message_h
#define Message_h

#include <vector>
#include <string>

typedef std::vector<std::string> MessageMap;

class Message
{
  public:
    inline Message() {;}
    inline Message(const char* msg_s) { 
      fMessageS = std::string(msg_s);
      fMessage = ToObject();
    }
    inline Message(const char* key, const char* value) { SetKeyValue(key, value); }
    inline Message(std::string msg_s) { fMessageS = msg_s; }
    inline Message(MessageMap msg_m) { fMessage = msg_m; }
    inline ~Message() {;}
    
    inline void SetKeyValue(const char* key, const char* value) {
      fMessage.push_back(std::string(key));
      fMessage.push_back(std::string(value));
      fMessageS = ToString();
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
