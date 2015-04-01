#include "Listener.h"

Listener::Listener(int port) :
  Socket(port), fIsConnected(false), fListenerId(-1)
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
    // Once connected we announce our presence to the server
    //SendMessage(Message(ADD_LISTENER, ""));
    
    // Then we wait for it to send us a connection acknowledgement + an id
    Message ack = FetchMessage();
    
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
  //if (!fIsConnected) return;
  try {
    SendMessage(Message(REMOVE_LISTENER, fListenerId), -1);
  } catch (Exception& e) {
    e.Dump();
  }
  try {
    Message ack = FetchMessage();
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
    //Listen(5);
    Message msg = FetchMessage();
    //msg.Dump();
  } catch (Exception& e) {
    //e.Dump();
    //exit(0);
  }
}
