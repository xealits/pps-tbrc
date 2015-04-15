#ifndef Messenger_h
#define Messenger_h

#include "Socket.h"

#define MAX_MESSAGE_ATTEMPTS 5 

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
    void DisconnectClient(int sid, MessageKey key, bool force=false);
    
    MessageKey Receive();
    void ProcessMessage(SocketMessage m, int sid);
    void Broadcast(Message m);
    inline void Send(const Message& m, int sid); 
    
  private:
    WebSocket* fWS;
    int fNumAttempts;
};

#endif

