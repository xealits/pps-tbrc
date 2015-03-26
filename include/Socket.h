#ifndef Socket_h
#define Socket_h

#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // internet address family (for sockaddr_in)
#include <arpa/inet.h> // definitions for internet operations
#include <netdb.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

#include <sstream>
#include <iostream>

#include "Exception.h"

#define SOCKET_ERROR(x) 10000+x
#define MAX_WORD_LENGTH 1000

/**
 * General object providing all useful method to connect/bind/send/receive
 * information through system sockets.
 *
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 23 Mar 2015
 */
class Socket
{
  public:
    inline Socket() {;}
    Socket(int port);
    virtual ~Socket();
    
    /// Retrieve the port used for this socket
    inline int GetPort() const { return fPort; }
  
    /**
     * Set the socket to accept connections from clients
     * \brief Accept connections from outside
     */
    void AcceptConnections(Socket& socket) const;
    
  protected:
    /**
     * Launch all mandatory operations to set the socket to be used
     * \brief Start the socket
     * \return Success of the operation
     */
    bool Start();
    /// Terminates the socket and all attached communications
    void Stop();
    /**
     * \brief Bind a name to a socket
     * \return Success of the operation
     */
    void Bind();
    void PrepareConnection();
    /**
     * Set the socket to listen to any message coming from outside
     * \brief Listen to incoming messages
     */
    void Listen(int maxconn);
    
    /**
     * \brief Send a message on a socket
     */
    void SendMessage(std::string command);
    /**
     * \brief Receive a message from a socket
     * \return Received message as a std::string
     */
    std::string FetchMessage();
    
    /**
     * A file descriptor for this socket, if \a Create was performed beforehand.
     */
    int fSocketId;
    int fPort;
    char fBuffer[MAX_WORD_LENGTH];
    
  private:
    /**
     * \brief Create an endpoint for communication
     */
    void Create();
    void Configure();
    
    struct sockaddr_in6 fAddress;
};

#endif
