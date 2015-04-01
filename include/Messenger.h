#ifndef Messenger_h
#define Messenger_h

#include "Socket.h"

#include <set>

typedef std::set<int> ListenersList;

/**
 * Messenger/broadcaster object used by the server to send/receive commands from
 * the clients/listeners.
 *
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 23 Mar 2015
 */
class Messenger : public Socket
{
  public:
    Messenger();
    Messenger(int port);
    ~Messenger();

    bool Connect();
    void Disconnect();
    
    MessageKey Receive();
    void ProcessMessage(Message& m);
    void Broadcast(Message m);

};

#endif

