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
  
    void Send(const Message& m) const;
    void Receive();
    virtual void ParseMessage(const SocketMessage& m) {;}
  
  private:
    bool Announce();
  
    int fListenerId;
    bool fIsConnected;
};

#endif
