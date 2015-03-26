#ifndef Listener_h
#define Listener_h

#include <string>

#include "Socket.h"

class Listener : public Socket
{
 public:
  Listener(int port);
  ~Listener();

  bool Connect();
  bool Disconnect();
  
 private:
  bool Announce();
  
  std::string fListenerId;
  bool fIsConnected;
};

#endif
