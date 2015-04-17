#ifndef Messenger_h
#define Messenger_h

#include "Socket.h"

typedef std::set<int> ListenersList;

struct ListenerInfo
{
  std::string name;
  int type;
};

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
    
    void AddClient();
    /**
     * Ask to a client to disconnect from this socket
     * \brief Disconnect a client
     * \param[in] sid Unique identifier of the client to disconnect
     * \param[in] key Key to the message to transmit for disconnection
     * \param[in] force Do we need to force the client out of this socket ?
     */
    void DisconnectClient(int sid, MessageKey key, bool force=false);
    
    /**
     * \brief Send any type of message to any client
     * \param[in] m Message to transmit
     * \param[in] sid Unique identifier of the client on this socket
     */
    inline void Send(const Message& m, int sid) const;
    /**
     * \brief Handle a message reception from a client
     * \return The key to the message received if successfully parsed
     */
    MessageKey Receive();
    /**
     * \brief Process a message received from the socket
     * \param[in] Unique identifier of the client sending the message
     */
    void ProcessMessage(SocketMessage m, int sid);
    /**
     * \brief Emit a message to all clients connected through the socket
     * \param[in] m Message to transmit
     */
    void Broadcast(const Message& m) const;
    
  private:
    WebSocket* fWS;
    int fNumAttempts;
    std::vector<ListenerInfo> fListenersInfo;
};

#endif

