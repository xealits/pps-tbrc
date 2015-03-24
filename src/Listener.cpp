#include "Listener.h"
#include <iostream>

Listener::Listener(int port):
  fPort(port), fSocketId(-1)
{
  memset(&fAddr, 0, sizeof(fAddr));
}

Listener::~Listener()
{
  if (fSocketId!=-1) close(fSocketId);
}

bool
Listener::Connect()
{
  // creating the socket
  fSocketId = socket(AF_INET, SOCK_STREAM, 0);
  if (fSocketId==-1) {
    std::cout << __PRETTY_FUNCTION__ << " cannot create socket!" << std::endl;
    return false;
  }
  
  int on = 1;
  int sockopt_result = setsockopt(fSocketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));
  if (sockopt_result!=0) {
    std::cout << __PRETTY_FUNCTION__ << " cannot modify socket options (result=" << sockopt_result << ")!" << std::endl;
    return false;
  }

  // binding to the port
  fAddr.sin_family = AF_INET;
  fAddr.sin_addr.s_addr = INADDR_ANY;
  fAddr.sin_port = htons(fPort);

  int connect_result = connect(fSocketId, (struct sockaddr*)&fAddr, sizeof(fAddr));
  if (connect_result!=0) {
    std::cout << __PRETTY_FUNCTION__ << " cannot connect to  socket (result=" << connect_result << ")!" << std::endl;
    close(fSocketId);
    return false;
  }

  std::string s("connect");
  int send_status = send(fSocketId, s.c_str(), s.size(), MSG_NOSIGNAL);
  if (send_status==-1) return false;

  std::cout << __PRETTY_FUNCTION__ << " connected to socket at port " << fPort << std::endl;
  return true;
}

