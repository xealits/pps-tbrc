#ifndef Listener_h
#define Listener_h

#include <string>

#include "Socket.h"

/**
 * Listener/client object used by the server to send/receive commands from
 * the messenger/broadcaster.
 *
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 24 Mar 2015
 */
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
