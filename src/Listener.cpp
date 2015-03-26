#include "Listener.h"

Listener::Listener(int port) :
  Socket(port), fIsConnected(false), fListenerId("")
{}

Listener::~Listener()
{}

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
  //return Announce();
  return true;
}
  
bool
Listener::Announce()
{
  try {
    // Once connected we announce our presence to the server
    SendMessage("new_client");
    
    // Then we wait for it to send us a connection acknowledgement + an id
    fListenerId = FetchMessage();
    
    if (fListenerId.empty()) return false;
    
  } catch (Exception& e) {
    e.Dump();
    return false;
  }
  
  std::cout << __PRETTY_FUNCTION__ << " connected to socket at port " << GetPort() << ", received id \"" << fListenerId << "\""<< std::endl;
  return true;
}

bool
Listener::Disconnect()
{
  if (!fIsConnected) return false;  
}
