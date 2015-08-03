#include "Socket.h"

Socket::Socket(int port) :
  fSocketId(-1), fPort(port)
{
  memset(fBuffer, 0, MAX_WORD_LENGTH);
  memset(&fAddress, 0, sizeof(fAddress));
  
  // Clear the master and temp file descriptor sets
  FD_ZERO(&fMaster);
  FD_ZERO(&fReadFds);
}

Socket::~Socket()
{}

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
  fSocketId = socket(AF_INET, SOCK_STREAM, 0);
  if (fSocketId==-1) {
    throw Exception(__PRETTY_FUNCTION__, "Cannot create socket!", Fatal, SOCKET_ERROR(errno));
  }
}

void
Socket::Configure()
{
  const int on = 1/*, off = 0*/;
  struct timeval timeout;      
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;
  if (setsockopt(fSocketId, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout))!=0
   or setsockopt(fSocketId, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout))!=0
   or setsockopt(fSocketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on))!=0) {
    throw Exception(__PRETTY_FUNCTION__, "Cannot modify socket options", Fatal, SOCKET_ERROR(errno));    
  }
}

void
Socket::Bind()
{
  // binding the socket
  fAddress.sin_family = AF_INET;
  fAddress.sin_addr.s_addr = INADDR_ANY;
  fAddress.sin_port = htons(fPort);

  int bind_result = bind(fSocketId, (struct sockaddr*)&fAddress, sizeof(fAddress));
  if (bind_result==-1) {
    Stop();
    throw Exception(__PRETTY_FUNCTION__, "Cannot bind socket!", Fatal, SOCKET_ERROR(errno));
  }
}

void
Socket::PrepareConnection()
{
  fAddress.sin_family = AF_INET;
  fAddress.sin_port = htons(fPort);

  if (connect(fSocketId, (struct sockaddr*)&fAddress, sizeof(fAddress))!=0) {
    Stop();
    if (errno==111) { // Connection refused (likely that the server is non-existent)
      std::ostringstream os;
      os << "Messenger is not reachable through sockets!" << std::endl
         << "\tCheck that it is properly launched on the central" << std::endl
         << "\tmachine!";
      throw Exception(__PRETTY_FUNCTION__, os.str(), Fatal, SOCKET_ERROR(errno));
    }
    throw Exception(__PRETTY_FUNCTION__, "Cannot connect to socket!", Fatal, SOCKET_ERROR(errno));
  }
}

void
Socket::AcceptConnections(Socket& socket)
{
  // Now we can start accepting connections from clients
  socklen_t len = sizeof(fAddress);
  socket.SetSocketId(accept(fSocketId, (struct sockaddr*)&fAddress, &len));
  std::ostringstream o; o << "New client with # " << socket.GetSocketId();
  PrintInfo(o.str());
  if (socket.GetSocketId()<0) {
    throw Exception(__PRETTY_FUNCTION__, "Cannot accept client!", JustWarning, SOCKET_ERROR(errno));
  }
  // Add to master set
  FD_SET(socket.GetSocketId(), &fMaster);
  fSocketsConnected.insert(std::pair<int,SocketType>(socket.GetSocketId(), CLIENT));
}

void
Socket::SelectConnections()
{
  if (fSocketsConnected.size()==0) {
    throw Exception(__PRETTY_FUNCTION__, "The messenger socket is not registered to the sockets list!", Fatal);
  }
  int highest = fSocketsConnected.rbegin()->first; // last one in the set
  if (select(highest+1, &fReadFds, NULL, NULL, NULL)==-1) {
    throw Exception(__PRETTY_FUNCTION__, "Unable to select the connection!", Fatal, SOCKET_ERROR(errno));
  }
}

void
Socket::Listen(int maxconn)
{
  if (listen(fSocketId, maxconn)!=0) {
    Stop();
    throw Exception(__PRETTY_FUNCTION__, "Cannot listen on socket!", JustWarning, SOCKET_ERROR(errno));
  }
  // Add the messenger to the master set
  FD_SET(fSocketId, &fMaster);
  fSocketsConnected.insert(std::pair<int,SocketType>(fSocketId, MASTER));
}

void
Socket::SendMessage(Message message, int id) const
{
  if (id<0) id = fSocketId;
  std::string message_s = message.GetString();
  usleep(1000);
  if (send(id, message_s.c_str(), message_s.size(), MSG_NOSIGNAL)<=0) {
    throw Exception(__PRETTY_FUNCTION__, "Cannot send message!", JustWarning, SOCKET_ERROR(errno));
  }
}

Message
Socket::FetchMessage(int id) const
{
  // At first we prepare the buffer to be filled
  char buf[MAX_WORD_LENGTH];
  memset(buf, 0, MAX_WORD_LENGTH);
  
  if (id<0) id = fSocketId;
 
  size_t num_bytes = recv(id, buf, MAX_WORD_LENGTH, 0);
  if (num_bytes<0) {
    throw Exception(__PRETTY_FUNCTION__, "Cannot read answer from receiver!", JustWarning, SOCKET_ERROR(errno));
    //...
  }
  else if (num_bytes==0) {
    std::ostringstream o; o << "Socket " << id << " just got disconnected";
    throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning, 11000);
  }
 
  std::string out(buf);
  return Message(out.substr(0, num_bytes));
}

void
Socket::DumpConnected() const
{
  std::ostringstream os;
  os << " List of sockets connected: ";
  for (SocketCollection::const_iterator it=fSocketsConnected.begin(); it!=fSocketsConnected.end(); it++) {
    os << " " << it->first;
    if (it->second) os << " (web)";
  }
  PrintInfo(os.str());
}
