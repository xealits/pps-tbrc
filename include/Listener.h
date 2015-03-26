#ifndef Listener_h
#define Listener_h

#include <string>

#include "Socket.h"

class Listener : public Socket
{
  public:
    inline Listener() {;}
    Listener(int port);
    ~Listener();

    bool Connect();
    void Disconnect();
  
    void Receive();
  
  private:
    bool Announce();
  
    std::string fListenerId;
    bool fIsConnected;
};

#endif
