#ifndef Messenger_h
#define Messenger_h

#include "Socket.h"

#include <unistd.h>
#include <sys/wait.h>

/**
 * Messenger/broadcaster object used by the server to send/receive commands from
 * the clients/listeners.
 * \brief Base master object for the socket
 *
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 23 Mar 2015
 * \ingroup Socket
 */
class Messenger : public Socket
{
  public:
    /// Build a void master object or socket actor
    Messenger();
    /// Build a master object to control the socket
    Messenger(int port);
    ~Messenger();

    /**
     * Connect this master to the socket for clients to be able to bind.
     * \brief Connect the master to the socket
     */
    bool Connect();
    /**
     * Remove this master from the socket, thus disconnecting automatically the
     * clients connected.
     * \brief Remove the master and destroy the socket
     */
    void Disconnect();
        
    /**
     * \brief Send any type of message to any client
     * \param[in] m Message to transmit
     * \param[in] sid Unique identifier of the client on this socket
     */
    void Send(const Message& m, int sid) const;
    /**
     * \brief Send any type of message to all clients of one type
     * \param[in] type Client type
     * \param[in] m Message to transmit
     */
    inline void SendAll(const Socket::SocketType& type, const Message& m) const {
      for (SocketCollection::const_iterator it=fSocketsConnected.begin(); it!=fSocketsConnected.end(); it++) {
        if (it->second==type) Send(m, it->first);
      }
    }
    inline void SendAll(const Socket::SocketType& type, const Exception& e) const {
      for (SocketCollection::const_iterator it=fSocketsConnected.begin(); it!=fSocketsConnected.end(); it++) {
        if (it->second==type) Send(SocketMessage(EXCEPTION, e.OneLine()), it->first);
      }
    }
    /// Handle a message reception from a client
    void Receive();
    /**
     * \brief Emit a message to all clients connected through the socket
     * \param[in] m Message to transmit
     */
    void Broadcast(const Message& m) const;
    /// Start the data acquisition
    void StartAcquisition();
    void StopAcquisition();
    /// Socket actor type retrieval method
    inline SocketType GetType() const { return MASTER; }
  private:
    /**
     * Add one client to the list of socket actors to monitor for message
     * retrieval/submission.
     * \brief Add a client to listen to
     */
    void AddClient();
    /**
     * Ask to a client to disconnect from this socket.
     * \brief Disconnect a client
     * \param[in] sid Unique identifier of the client to disconnect
     * \param[in] key Key to the message to transmit for disconnection
     * \param[in] force Do we need to force the client out of this socket ?
     */
    void DisconnectClient(int sid, MessageKey key, bool force=false);
    void SwitchClientType(int sid, Socket::SocketType type);
    /**
     * \brief Process a message received from the socket
     * \param[in] Unique identifier of the client sending the message
     */
    void ProcessMessage(SocketMessage m, int sid);
    WebSocket* fWS;
    int fNumAttempts;
    pid_t fPID;
    
    int fStdoutPipe[2], fStderrPipe[2];
};

#endif

