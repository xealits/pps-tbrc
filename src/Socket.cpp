#include "Socket.h"

Socket::Socket(int port) :
  fSocketId(-1), fPort(port)
{
  memset(fBuffer, 0, MAX_WORD_LENGTH);
  memset(&fAddress, 0, sizeof(fAddress));  
}

Socket::~Socket()
{
  Stop();
}

bool
Socket::Start()
{
  try {
    Create();
    Configure();
  } catch (Exception& e) {
    e.Dump();
    return false;
  }
  
  return true;
}

void
Socket::Stop()
{
  if (fSocketId==-1) return;
  close(fSocketId);
  fSocketId = -1;
}

void
Socket::Create()
{
  fSocketId = socket(AF_INET6, SOCK_STREAM, 0);
  if (fSocketId==-1) {
    throw Exception(__PRETTY_FUNCTION__, "Cannot create socket !", Fatal, SOCKET_ERROR(errno));
  }
}

void
Socket::Configure()
{
  const int on = 1, off = 0;
  if (setsockopt(fSocketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on))!=0) {
    throw Exception(__PRETTY_FUNCTION__, "Cannot modify socket options", Fatal, SOCKET_ERROR(errno));
  }
}

void
Socket::Bind()
{
  // binding the socket
  fAddress.sin6_family = AF_INET6;
  fAddress.sin6_addr = in6addr_any;
  fAddress.sin6_port = htons(fPort);

  int bind_result = bind(fSocketId, (struct sockaddr*)&fAddress, sizeof(fAddress));
  if (bind_result!=0) {
    Stop();
    throw Exception(__PRETTY_FUNCTION__, "Cannot bind socket !", Fatal, SOCKET_ERROR(errno));
  }
}

void
Socket::PrepareConnection()
{
  fAddress.sin6_family = AF_INET6;
  fAddress.sin6_port = htons(fPort);

  if (connect(fSocketId, (struct sockaddr*)&fAddress, sizeof(fAddress))!=0) {
    Stop();
    throw Exception(__PRETTY_FUNCTION__, "Cannot connect to socket !", Fatal, SOCKET_ERROR(errno));
  }
}

void
Socket::AcceptConnections(Socket& socket) const
{
  // now we can start accepting connections from clients
  socklen_t len = sizeof(fAddress);
  socket.fSocketId = accept(fSocketId, (struct sockaddr*)&fAddress, &len);
  if (socket.fSocketId<0) {
    throw Exception(__PRETTY_FUNCTION__, "Cannot accept client !", JustWarning, SOCKET_ERROR(errno));
  }
}

void
Socket::Listen(int maxconn)
{
  if (listen(fSocketId, maxconn)!=0) {
    Stop();
    throw Exception(__PRETTY_FUNCTION__, "Cannot listen on socket !", JustWarning, SOCKET_ERROR(errno));
  }
}

void
Socket::SendMessage(Message message)
{
  std::string message_s = message.String();
  message.Dump();
  std::cout << "Message to send: " << message_s << std::endl;
  if (send(fSocketId, message_s.c_str(), message_s.size(), MSG_NOSIGNAL)<=0) {
    throw Exception(__PRETTY_FUNCTION__, "Cannot send message !", JustWarning, SOCKET_ERROR(errno));
  }
}

Message
Socket::FetchMessage()
{
  // at first we prepare the buffer to be filled
  char buf[MAX_WORD_LENGTH];
  memset(buf, 0, MAX_WORD_LENGTH);
  
  switch (recv(fSocketId, buf, MAX_WORD_LENGTH, 0)) {
    case -1:
      throw Exception(__PRETTY_FUNCTION__, "Cannot read answer from server !", JustWarning, SOCKET_ERROR(errno));
    case 0:
      //std::cout << __PRETTY_FUNCTION__ << " client disconnected !" << std::endl;
      return buf;
    default:
      break;
  }
  
  if (buf==0 or buf=="" or buf=='\0')
    throw Exception(__PRETTY_FUNCTION__, "Empty message received !", JustWarning, SOCKET_ERROR(errno));

  if (strstr(buf, ":")==NULL) // no column -> invalid key:value pattern
    throw Exception(__PRETTY_FUNCTION__, "Invalid message received !", JustWarning, SOCKET_ERROR(errno));
    
  std::cout << "---> (" << buf << ") received" << std::endl;
  return Message(buf).Object();
}
