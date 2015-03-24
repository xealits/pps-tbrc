#include "Messenger.h"
#include <iostream>

Messenger::Messenger(int port):
  fPort(port), fSocketId(-1), fAcceptId(-1)
{
  memset(buf, 0, 1000);
  std::cout << __PRETTY_FUNCTION__ << " new Messenger at port " << fPort << std::endl;
}

Messenger::~Messenger()
{
  if (fSocketId!=-1) close(fSocketId);
  if (fAcceptId!=-1) close(fSocketId);
}

bool
Messenger::Connect()
{
  // creating the socket
  fSocketId = socket(AF_INET, SOCK_STREAM, 0);
  if (fSocketId==-1) {
    std::cout << __PRETTY_FUNCTION__ << " cannot create socket!" << std::endl;
    return false;
  }

  // FIXME
  int on = 1;
  int sockopt_result = setsockopt(fSocketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof (on));
  if (sockopt_result!=0) {
    std::cout << __PRETTY_FUNCTION__ << " cannot modify socket options (result=" << sockopt_result << ")!" << std::endl;
    return false;
  }
  std::cout << " --> socket at " << fSocketId << std::endl;
  //

  // binding the socket
  struct sockaddr_in server;
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(fPort);

  int bind_result = bind(fSocketId, (struct sockaddr*)&server, sizeof(server));
  if (bind_result!=0) {
    std::cout << __PRETTY_FUNCTION__ << " cannot bind socket (result=" << bind_result << ")!" << std::endl;
    close(fSocketId);
    return false;
  }

  // listening...
  int listen_result = listen(fSocketId, 20);  
  if (listen_result!=0) {
    std::cout << __PRETTY_FUNCTION__ << " cannot listen on socket (result=" << listen_result << ")!" << std::endl;
    close(fSocketId);  
    return false;
  }

  listen(fSocketId, 5);

  // return a true iff everything went fine !
  return true;
}

bool
Messenger::Receive()
{
  struct sockaddr_in client;
  memset(&client, 0, sizeof(client));
  socklen_t len = sizeof(client);

  // now we can start accepting connections from clients
  fAcceptId = accept(fSocketId, (struct sockaddr*)&client, &len);
  if (fAcceptId<0) {
    std::cout << __PRETTY_FUNCTION__ << " cannot accept client (result=" << fAcceptId << ")!" << std::endl;
    close(fSocketId);
    return false;
  }

  int receive_status = recv(fAcceptId, buf, 1000, 0);
  switch (receive_status) {
    case -1:
      std::cout << __PRETTY_FUNCTION__ << " cannot read from client !" << std::endl;
      return false;
    case 0:
      //std::cout << __PRETTY_FUNCTION__ << " client disconnected !" << std::endl;
      return false;
    default:
      break;
  }
  
  std::cout << __PRETTY_FUNCTION__ << " received \"" << buf << "\"" << std::endl;
  memset(buf, 0, 1000);
  return true;
}
