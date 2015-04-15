#include "Messenger.h"

Messenger::Messenger() :
  Socket(-1), fWS(0), fNumAttempts(0)
{}

Messenger::Messenger(int port) :
  Socket(port), fWS(0), fNumAttempts(0)
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

void
Messenger::DisconnectClient(int sid, MessageKey key, bool force)
{
  bool ws;
  try {
    ws = IsWebSocket(sid);
  } catch (Exception& e) {
    e.Dump();
    return;
  }
  std::ostringstream o; o << "Disconnecting client # " << sid;
  if (ws) o << " (web socket)";
  Exception(__PRETTY_FUNCTION__, o.str(), Info).Dump();
  if (ws) {
    try {
      Send(SocketMessage(key, ""), sid);
      //std::ostringstream o; o << 0xFF << 0x00;
      //Send(Message(o.str()), sid);
    } catch (Exception& e) {
      e.Dump();
      if (e.ErrorNumber()==10032 or force) {
        fSocketsConnected.erase(std::pair<int,bool>(sid, ws));
        FD_CLR(sid, &fMaster);
        return;
      }
    }
  }
  else {
    Messenger* mes = new Messenger;
    mes->SetSocketId(sid);
    mes->Stop();
    delete mes;
  }
  fSocketsConnected.erase(std::pair<int,bool>(sid, ws));
  FD_CLR(sid, &fMaster);
}

void
Messenger::Send(const Message& m, int sid)
{
  bool ws = false;
  try {
    ws = IsWebSocket(sid);
  } catch (Exception& e) {
    e.Dump();
  }
  if (ws) SendMessage(HTTPMessage(fWS, m, EncodeMessage), sid);
  else SendMessage(m, sid);
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
        if (message.IsFromWeb()) {
          // Feed the handshake to the WebSocket object
          fWS->parseHandshake((unsigned char*)message.GetString().c_str(), message.GetString().size());
          Send(Message(fWS->answerHandshake()), mess.GetSocketId());
          // From now on handle a WebSocket connection
          fSocketsConnected.erase(std::pair<int,bool>(mess.GetSocketId(), false));
          fSocketsConnected.insert(std::pair<int,bool>(mess.GetSocketId(), true));
          // Send the client's unique identifier
          Send(SocketMessage(SET_LISTENER_ID, mess.GetSocketId()), mess.GetSocketId());
          return WEBSOCKET_KEY;
        }
        else {
          // if not a WebSocket connection
          Send(SocketMessage(SET_LISTENER_ID, mess.GetSocketId()), mess.GetSocketId());
          return ADD_LISTENER;
        }
      } catch (Exception& e) {
        e.Dump();
      }
      return MessageKey();
    }
    
    // Handle data from a client
    Message message;
    try {
      message = FetchMessage(sid->first);
    } catch (Exception& e) {
      if (e.ErrorNumber()==11000) {
        DisconnectClient(sid->first, THIS_LISTENER_DELETED);
      }
    }
    SocketMessage m;
    if (sid->second) {
      HTTPMessage msg(fWS, message, DecodeMessage);
      try {
        m = SocketMessage(msg);
      } catch (Exception& e) {;}
    }
    else m = SocketMessage(message.GetString());

    // Message was successfully decoded
    fNumAttempts = 0;
    
    try {
      ProcessMessage(m, sid->first);
    } catch (Exception& e) {
      e.Dump();
    }
    return message.GetKey();
  }
  
  return INVALID_KEY;
}

void
Messenger::ProcessMessage(SocketMessage m, int sid)
{
  if (m.GetKey()==REMOVE_LISTENER) {
    MessageKey key;
    
    if (m.GetIntValue()==GetSocketId()) {
      std::ostringstream o;
      o << "Some client (id=" << sid << ") asked for this master's disconnection!"
        << "\n\tIgnoring this request...";
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
      return;
    }
    
    if (sid==m.GetIntValue()) key = THIS_LISTENER_DELETED;
    else key = OTHER_LISTENER_DELETED;
    DisconnectClient(m.GetIntValue(), key);
  }
  else if (m.GetKey()==PING_LISTENER) {
    int toping = m.GetIntValue();
    Send(SocketMessage(PING_LISTENER), toping);
    SocketMessage msg;
    do { msg = FetchMessage(toping); } while (msg.GetKey()!=PING_ANSWER);
    Send(SocketMessage(PING_ANSWER, msg.GetValue()), sid);
  } 
  else if (m.GetKey()==GET_LISTENERS) {
    int i = 0;
    std::ostringstream os;
    for (SocketCollection::const_iterator it=fSocketsConnected.begin(); it!=fSocketsConnected.end(); it++, i++) {
      if (i!=0) os << ";";
      os << it->first;
    }
    Send(SocketMessage(LISTENERS_LIST, os.str()), sid);
  }
  else if (m.GetKey()==WEB_GET_LISTENERS) {
    int i = 0, type;
    std::ostringstream os;
    for (SocketCollection::const_iterator it=fSocketsConnected.begin(); it!=fSocketsConnected.end(); it++, i++) {
      if (it->first==GetSocketId()) type = 0; // master (us)
      else if (!it->second) type = 1; // regular socket
      else type = 2; // web socket
      
      if (i!=0) os << ";";
      
      os << it->first << ",";
      if (it->first==GetSocketId()) os << "Master,";
      else os << "socket_" << it->first << ",";
      
      os << type;
    }
    try {
      Send(SocketMessage(LISTENERS_LIST, os.str()), sid);
    } catch (Exception& e) {
      e.Dump();
    }
  }
}

void
Messenger::Broadcast(Message m)
{
  try {
    for (SocketCollection::const_iterator sid=fSocketsConnected.begin(); sid!=fSocketsConnected.end(); sid++) {
      if (sid->first==GetSocketId()) continue;
      Send(m, sid->first);
    }
  } catch (Exception& e) {
    e.Dump();
  }
}
