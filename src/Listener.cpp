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
    SendMessage(Message("new_client", ""));
    
    // Then we wait for it to send us a connection acknowledgement + an id
    fListenerId = FetchMessage().GetValue();
    
    if (fListenerId.empty()) return false;
    
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
  std::cout << "->-> Disconnecting" << std::endl;
  //if (!fIsConnected) return;
  try {
    SendMessage(Message("terminate_client", fListenerId.c_str()));
  } catch (Exception& e) {
    e.Dump();
  }
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
