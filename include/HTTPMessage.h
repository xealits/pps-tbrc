#ifndef HTTPMessage_h
#define HTTPMessage_h

#include "Message.h"
#include "WebSocket/WebSocket.h"

#define MAX_WS_MESSAGE_SIZE 5000

class HTTPMessage : public Message
{
  public:
    inline HTTPMessage(WebSocket* ws, Message m, bool encode) : Message(m), fWS(ws) {
      if (encode) Encode();
      else Decode();
    }
    inline HTTPMessage(WebSocket* ws, const char* msg, bool encode) : Message(msg), fWS(ws) {
      if (encode) Encode();
      else Decode();
    }
    
    inline void Decode() {
      unsigned char outbuf[MAX_WS_MESSAGE_SIZE];
      memset(outbuf, 0, MAX_WS_MESSAGE_SIZE);
      int outbufsize;
      fWS->getFrame((unsigned char*)fString.c_str(), fString.size(), outbuf, MAX_WS_MESSAGE_SIZE, &outbufsize);
      std::string out((const char*)outbuf);
      std::cout << "before " << outbufsize << " decoding: " << out << std::endl;
      fString = out.substr(0, outbufsize);
    }
    
    inline void Encode() {
      unsigned char outbuf[MAX_WS_MESSAGE_SIZE];
      memset(outbuf, 0, MAX_WS_MESSAGE_SIZE);
      int size = fWS->makeFrame(TEXT_FRAME, (unsigned char*)fString.c_str(), fString.size(), outbuf, MAX_WS_MESSAGE_SIZE);
      std::string out((const char*)outbuf);
      fString = out.substr(0, size);
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
