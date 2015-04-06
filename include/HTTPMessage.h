#ifndef HTTPMessage_h
#define HTTPMessage_h

#include "Message.h"
//#include "libwebsocket.h"
//#include "websocketpp/http/request.hpp"
#include "WebSocket/WebSocket.h"

class HTTPMessage : public Message
{
  public:
    inline HTTPMessage(Message& m, bool init=false): Message(m), fStatus(-1) {
      if (init) fWS.parseHandshake((unsigned char*)fString.c_str(), fString.size());
    }
    inline HTTPMessage(std::string msg, bool init=false) : Message(msg) {
      HTTPMessage(msg.c_str());
    }
    inline HTTPMessage(const char* msg, bool init=false) : Message(msg), fStatus(501) {
      //websocketpp::http::parser::parser p;
      //websocketpp::http::parser::request req;
      //std::cout << "--> " << req.consume(msg, sizeof(msg)) << " -> [" << req.get_body() << "]" << std::endl;
      //std::ostringstream s; s << msg;
      //std::cout << "-> " << req.parse_complete() << ", " << 
      if (init) fWS.parseHandshake((unsigned char*)fString.c_str(), fString.size());
    }
    inline MessageKey GetKey() const { return WEBSOCKET_KEY; }
    
    inline HTTPMessage AnswerHandshake() { return HTTPMessage(fWS.answerHandshake()); }
    
    inline void Dump(std::ostream& os=std::cout) const {
      os << "============= Web-socket Message dump =============" << std::endl
         << " Resource: " << fWS.resource << std::endl
         << " Host:     " << fWS.host << std::endl
         << " Origin:   " << fWS.origin << std::endl
         << " Protocol: " << fWS.protocol << std::endl
         << " Key:      " << fWS.key << std::endl
         << "===================================================" << std::endl;      
    }
    
  private:
    int fStatus;
    WebSocket fWS;
};

#endif
