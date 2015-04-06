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
    Broadcast(SocketMessage(MASTER_DISCONNECT, ""));
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
        Message message = FetchMessage(mess.GetSocketId());
        if (message.IsFromWeb()) {
          // handle a WebSocket connection
          HTTPMessage m(message, true);
          SendMessage(m.AnswerHandshake(), mess.GetSocketId());
          std::cout << "new web connection!" << std::endl;
          return WEBSOCKET_KEY;
        }
        else {
          // if not a WebSocket connection
          SendMessage(SocketMessage(SET_LISTENER_ID, mess.GetSocketId()), mess.GetSocketId());
        }
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
  return INVALID_KEY;
}

void
Messenger::ProcessMessage(Message& m)
{
  if (m.IsFromWeb()) {
    HTTPMessage mess(m);
    std::cout << "new message from web!" << std::endl;
    try {
      SendMessage(mess.AnswerHandshake());
    } catch (Exception& e) {
      e.Dump();
      cout<<"haha"<<endl;
    }
  }
  else {
    SocketMessage mess(m);
    Messenger* mes;
    switch (mess.GetKey()) {
      case REMOVE_LISTENER:
        fSocketsConnected.erase(mess.GetIntValue());
        mes = new Messenger;
        mes->SetSocketId(mess.GetIntValue());
        mes->Stop();
        FD_CLR(mess.GetIntValue(), &fMaster);
        delete mes;
        break;
      
      default:
        return;
    }
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
