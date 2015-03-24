#ifndef Listener_h
#define Listener_h

#include <sstream>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

class Listener
{
 public:
  Listener(int port);
  ~Listener();

  bool Connect();

 private:
  int fPort;
  int fSocketId;
  sockaddr_in fAddr;
};

#endif
