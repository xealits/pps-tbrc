#ifndef Messenger_h
#define Messenger_h

#include "Socket.h"

#include <list>

typedef enum
{
  INVALID,
  NEW_LISTENER,
  DEL_LISTENER,
  LISTENER_MESSAGE
} message_t;

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
    inline Messenger() {;}
    Messenger(int port);
    ~Messenger();

    bool Connect();
    message_t Receive();
    void Broadcast(std::string message);

  private:
    std::list<int> fListeners;
};

#endif
