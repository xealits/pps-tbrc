#include "Messenger.h"

Messenger::Messenger() :
  Socket(-1), fWS(0)
{}

Messenger::Messenger(int port) :
  Socket(port), fWS(0)
{
  std::cout << __PRETTY_FUNCTION__ << " new Messenger at port " << GetPort() << std::endl;
  fWS = new WebSocket;
}

Messenger::~Messenger()
{
  Disconnect();
  if (fWS) delete fWS;
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
    
    if (!FD_ISSET(sid->first, &fReadFds)) continue;
    
    // First check if we need to handle new connections
    if (sid->first==GetSocketId()) {
      Messenger mess;
      try {
        AcceptConnections(mess);
        Message message = FetchMessage(mess.GetSocketId());
        //message.Dump();
        if (message.IsFromWeb()) {
          fWS->parseHandshake((unsigned char*)message.GetString().c_str(), message.GetString().size());
          // handle a WebSocket connection
          //FIXME FIXME FIXME
          /*for (SocketCollection::iterator sid=fSocketsConnected.begin(); sid!=fSocketsConnected.end(); sid++) {
            if (sid->first==mess.GetSocketId()) sid->second = true; //sid = std::pair<int,bool>(mess.GetSocketId(), true);//
          }*/
          fSocketsConnected.erase(std::pair<int,bool>(mess.GetSocketId(), false));
          fSocketsConnected.insert(std::pair<int,bool>(mess.GetSocketId(), true));
          SendMessage(Message(fWS->answerHandshake()), mess.GetSocketId());
          return WEBSOCKET_KEY;
        }
        else {
          // if not a WebSocket connection
          SendMessage(SocketMessage(SET_LISTENER_ID, mess.GetSocketId()), mess.GetSocketId());
          return ADD_LISTENER;
        }
      } catch (Exception& e) {
        e.Dump();
      }
      return MessageKey();
    }
    
    // Handle data from a client
    std::cout << "receiving message from " << sid->first << " (is websocket? " << sid->second << ")" << std::endl;
    Message message = FetchMessage(sid->first);
    Message *m = 0;
    if (sid->second) m = new HTTPMessage(fWS, message);
    else m = new SocketMessage(message);
    std::cout << "haha" << std::endl;
    m->Dump();
    try {
      ProcessMessage(*m);
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
    std::cout << "new message from web!" << std::endl;
    HTTPMessage mess(fWS, m);
  }
  else {
    SocketMessage mess(m);
    Messenger* mes;
    switch (mess.GetKey()) {
      case REMOVE_LISTENER:
        fSocketsConnected.erase(std::pair<int,bool>(mess.GetIntValue(), false));
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
      if (sid->first==GetSocketId()) continue;
      SendMessage(m, sid->first);
    }
  } catch (Exception& e) {
    e.Dump();
  }
}
