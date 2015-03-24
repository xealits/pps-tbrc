#ifndef Messenger_h
#define Messenger_h

#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

class Messenger
{
 public:
  Messenger(int port);
  ~Messenger();

  bool Connect();
  bool Receive();

 private:
  char buf[1000];
  int fPort;
  int fSocketId;
  int fAcceptId;
};

#endif
