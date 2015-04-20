#ifndef Client_h
#define Client_h

#include <string>

#include "Socket.h"

/**
 * Client object used by the server to send/receive commands from
 * the messenger/broadcaster.
 *
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 24 Mar 2015
 */
class Client : public Socket
{
  public:
    inline Client() {;}
    Client(int port);
    ~Client();

    bool Connect();
    void Disconnect();
  
    inline void Send(const Message& m) const { SendMessage(m); }
    void Receive();
    
    virtual void ParseMessage(const SocketMessage& m) {;}
    virtual SocketType GetType() const { return CLIENT; }
  
  private:
    void Announce();
  
    int fClientId;
    bool fIsConnected;
};

#endif
