#include "Messenger.h"

Messenger::Messenger() :
  Socket(-1)
{}

Messenger::Messenger(int port) :
  Socket(port)
{
  std::cout << __PRETTY_FUNCTION__ << " new Messenger at port " << GetPort() << std::endl;
}

Messenger::~Messenger()
{
  Disconnect();
}

bool
Messenger::Connect()
{
  try {
    Start();
    Bind();
    Listen(10);
  } catch (Exception& e) {
    e.Dump();
    return false;
  }
  return true;
}

void
Messenger::Disconnect()
{
  if (fPort<0) return; // do not broadcast the death of a secondary messenger!
  try {
    Broadcast(Message(MASTER_DISCONNECT, ""));
  } catch (Exception& e) {
    e.Dump();
    throw Exception(__PRETTY_FUNCTION__, "Failed to broadcast the server disconnection status!", JustWarning, SOCKET_ERROR(errno));
  }
  Stop();
}

MessageKey
Messenger::Receive()
{
  // We start by copying the master file descriptors list to the
  // temporary list for readout
  fReadFds = fMaster;
  
  try {
    SelectConnections();
  } catch (Exception& e) {
    e.Dump();
    throw Exception(__PRETTY_FUNCTION__, "Impossible to select the connections!", Fatal);
  }
  
  // Looking for something to read!
  for (SocketCollection::const_iterator sid=fSocketsConnected.begin(); sid!=fSocketsConnected.end(); sid++) {
    
    if (!FD_ISSET(*sid, &fReadFds)) continue;
    
    // First check if we need to handle new connections
    if (*sid==GetSocketId()) {
      Messenger mess;
      try {
        AcceptConnections(mess);
        SendMessage(Message(SET_LISTENER_ID, mess.GetSocketId()), mess.GetSocketId());
      } catch (Exception& e) {
        e.Dump();
      }
      return MessageKey();
    }
    
    // Handle data from a client
    Message message = FetchMessage(*sid);
    //message.Dump();
    try {
      ProcessMessage(message);
    } catch (Exception& e) {
      e.Dump();
    }
    return message.GetKey();
  }
}

void
Messenger::ProcessMessage(Message& m)
{
  Messenger* mes;
  switch (m.GetKey()) {
    case REMOVE_LISTENER:
      fSocketsConnected.erase(m.GetIntValue());
      mes = new Messenger;
      mes->SetSocketId(m.GetIntValue());
      mes->Stop();
      FD_CLR(m.GetIntValue(), &fMaster);
      delete mes;
      break;
    
    default:
      return;
  }
}

void
Messenger::Broadcast(Message m)
{
  try {
    for (SocketCollection::const_iterator sid=fSocketsConnected.begin(); sid!=fSocketsConnected.end(); sid++) {
      if (*sid==GetSocketId()) continue;
      SendMessage(m, *sid);
    }
  } catch (Exception& e) {
    e.Dump();
  }
}
