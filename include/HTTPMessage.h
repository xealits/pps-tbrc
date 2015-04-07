#ifndef HTTPMessage_h
#define HTTPMessage_h

#include "Message.h"
#include "WebSocket/WebSocket.h"

#define MAX_WS_MESSAGE_SIZE 500

class HTTPMessage : public Message
{
  public:
    inline HTTPMessage(WebSocket* ws, Message& m) : Message(m), fWS(ws) {
      unsigned char outbuf[MAX_WS_MESSAGE_SIZE];
      memset(outbuf, 0, MAX_WS_MESSAGE_SIZE);
      int outbufsize;
      fWS->getFrame((unsigned char*)fString.c_str(), fString.size(), outbuf, MAX_WS_MESSAGE_SIZE, &outbufsize);
    }
    inline HTTPMessage(WebSocket* ws, const char* msg) : Message(msg), fWS(ws) {
      unsigned char outbuf[MAX_WS_MESSAGE_SIZE];
      memset(outbuf, 0, MAX_WS_MESSAGE_SIZE);
      int outbufsize;
      fWS->getFrame((unsigned char*)msg, sizeof(msg)/sizeof(const char*), outbuf, MAX_WS_MESSAGE_SIZE, &outbufsize);
    }
    inline MessageKey GetKey() const { return WEBSOCKET_KEY; }
        
    inline void Dump(std::ostream& os=std::cout) const {
      os << "============= Web-socket Message dump =============" << std::endl
         << " Resource: " << fWS->resource << std::endl
         << " Host:     " << fWS->host << std::endl
         << " Origin:   " << fWS->origin << std::endl
         << " Protocol: " << fWS->protocol << std::endl
         << " Key:      " << fWS->key << std::endl
         << "===================================================" << std::endl;      
    }
    
  private:
    WebSocket* fWS;
};

#endif
