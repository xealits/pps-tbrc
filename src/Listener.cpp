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
    
    ack.Dump();
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
    if (ack.GetKey()==LISTENER_DELETED) {
      fIsConnected = false;
    }
  } catch (Exception& e) {
    e.Dump();
  }
}

void
Listener::Receive()
{
  try {
    SocketMessage msg(FetchMessage());
    switch (msg.GetKey()) {
      case MASTER_DISCONNECT:
        throw Exception(__PRETTY_FUNCTION__, "Master disconnected!", Fatal);
      
      default:
        return;      
    }
  } catch (Exception& e) {
    e.Dump();
  }
}
