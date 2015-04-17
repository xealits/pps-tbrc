#include "Listener.h"

Listener::Listener(int port) :
  Socket(port), fListenerId(-1), fIsConnected(false)
{}

Listener::~Listener()
{
  if (fIsConnected) Disconnect();
}

bool
Listener::Connect()
{
  try {
    Start();
    PrepareConnection();
    Announce();
  } catch (Exception& e) { 
    e.Dump();
    return false;
  }

  fIsConnected = true;
  return true;
}
  
bool
Listener::Announce()
{
  try {
    // Once connected we send our request for connection
    SendMessage(SocketMessage(ADD_LISTENER, ""));
    
    // Then we wait for to the server to send us a connection
    // acknowledgement + an id
    SocketMessage ack(FetchMessage());
    
    switch (ack.GetKey()) {
    case SET_LISTENER_ID:
      fListenerId = ack.GetIntValue();
      break;
      
    case INVALID_KEY:
    default:
      throw Exception(__PRETTY_FUNCTION__, "Received an invalid answer from server", JustWarning);
    }
    
  } catch (Exception& e) {
    e.Dump();
    return false;
  }
  
  std::cout << __PRETTY_FUNCTION__ << " connected to socket at port " << GetPort() << ", received id \"" << fListenerId << "\""<< std::endl;
  return true;
}

void
Listener::Disconnect()
{
  std::cout << "===> Disconnecting the client from socket" << std::endl;
  if (!fIsConnected) return;
  try {
    SendMessage(SocketMessage(REMOVE_LISTENER, fListenerId), -1);
  } catch (Exception& e) {
    e.Dump();
  }
  try {
    SocketMessage ack(FetchMessage());
    if (ack.GetKey()==THIS_LISTENER_DELETED or ack.GetKey()==OTHER_LISTENER_DELETED) {
      fIsConnected = false;
    }
  } catch (Exception& e) {
    if (e.ErrorNumber()!=11000) // client has been disconnected
      e.Dump();
  }
}

void
Listener::Send(const Message& m) const
{
  SendMessage(m);
}

void
Listener::Receive()
{
  SocketMessage msg;
  try {
    msg = FetchMessage();
  } catch (Exception& e) {
    if (e.ErrorNumber()!=11000) // client has been disconnected
      e.Dump();
    else {
      throw Exception(__PRETTY_FUNCTION__, "Some other socket asked for this client's disconnection. Obtemperating...", Fatal);
    }
  }
  if (msg.GetKey()==MASTER_DISCONNECT) throw Exception(__PRETTY_FUNCTION__, "Master disconnected!", Fatal);
  else if (msg.GetKey()==OTHER_LISTENER_DELETED) 
    throw Exception(__PRETTY_FUNCTION__, "Some other socket asked for this client's disconnection. Obtemperating...", Fatal);
  else if (msg.GetKey()==PING_LISTENER) {
    ostringstream os; os << "Pong. I feel fine, thank you!";
    Send(SocketMessage(PING_ANSWER, os.str()));
    Exception(__PRETTY_FUNCTION__, "Got a ping, answering...", Info).Dump();
  } 
  else if (msg.GetKey()==LISTENERS_LIST) {
    VectorValue vals = msg.GetVectorValue();
    std::ostringstream o; o << "List of members on the socket:\n\t";
    int i = 0;
    for (VectorValue::const_iterator v=vals.begin(); v!=vals.end(); v++, i++) {
      if (i!=0) o << ", ";
      o << *v;
    }
    Exception(__PRETTY_FUNCTION__, o.str(), Info).Dump();
  }
  else {
    ParseMessage(msg);
  }
}
