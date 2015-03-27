#include "Listener.h"

Listener::Listener(int port) :
  Socket(port), fIsConnected(false), fListenerId("")
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
    SendMessage(Message(ADD_LISTENER, ""));
    
    // Then we wait for it to send us a connection acknowledgement + an id
    Message ack = FetchMessage();
    ack.Dump();
    if (ack.GetKey()==SET_LISTENER_ID) {
      fListenerId = ack.GetValue();
    }
    else {
      std::cout << __PRETTY_FUNCTION__ << " WARNING: received an invalid answer from server!" << std::endl;
      ack.Dump();
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
    SendMessage(Message(REMOVE_LISTENER, fListenerId.c_str()));
    Message ack = FetchMessage();
  } catch (Exception& e) {
    e.Dump();
  }
  fIsConnected = false;
}

void
Listener::Receive()
{
  try {
    //Listen(5);
    FetchMessage();
  } catch (Exception& e) {
    e.Dump();
  }
}
