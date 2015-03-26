#ifndef Messenger_h
#define Messenger_h

#include "Socket.h"

#include <list>

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
    bool Receive();

  private:
    std::list<int> fListeners;
};

#endif
