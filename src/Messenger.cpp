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
    Listen(20);
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
  SocketType type;
  try {
    type = GetSocketType(sid);
  } catch (Exception& e) {
    e.Dump();
    return;
  }
  std::ostringstream o; o << "Disconnecting client # " << sid;
  if (type==WEBSOCKET_CLIENT) o << " (web socket)";
  Exception(__PRETTY_FUNCTION__, o.str(), Info).Dump();
  if (type==WEBSOCKET_CLIENT) {
    try {
      Send(SocketMessage(key, sid), sid);
      //std::ostringstream o; o << 0xFF << 0x00;
      //Send(Message(o.str()), sid);
    } catch (Exception& e) {
      e.Dump();
      if (e.ErrorNumber()==10032 or force) {
        fSocketsConnected.erase(std::pair<int,SocketType>(sid, type));
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
  fSocketsConnected.erase(std::pair<int,SocketType>(sid, type));
  FD_CLR(sid, &fMaster);
}

void
Messenger::Send(const Message& m, int sid) const
{
  bool ws = false;
  try {
    ws = IsWebSocket(sid);
    Message tosend = (ws) ? HTTPMessage(fWS, m, EncodeMessage) : m;
    std::cout << "Sending a message to socket # " << sid << " (WS? " << ws << "):" << std::endl;
    tosend.Dump();
    SendMessage(tosend, sid);
  } catch (Exception& e) {
    e.Dump();
  }
}

void
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
      AddClient();
      return;
    }
    
    // Handle data from a client
    Message message;
    try {
      message = FetchMessage(sid->first);
    } catch (Exception& e) {
      if (e.ErrorNumber()==11000) {
        DisconnectClient(sid->first, THIS_CLIENT_DELETED);
      }
    }
    SocketMessage m;
    if (sid->second==WEBSOCKET_CLIENT) {
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
  }
  
  return;
}

void
Messenger::AddClient()
{
  Messenger mess;
  try {
    AcceptConnections(mess);
    Message message = FetchMessage(mess.GetSocketId());
    if (message.IsFromWeb()) {
      message.Dump();
      // Feed the handshake to the WebSocket object
      fWS->parseHandshake((unsigned char*)message.GetString().c_str(), message.GetString().size());
      Send(Message(fWS->answerHandshake()), mess.GetSocketId());
      // From now on handle a WebSocket connection
      SwitchClientType(mess.GetSocketId(), WEBSOCKET_CLIENT);
    }
    else {
      // if not a WebSocket connection
      SocketMessage m(message);
      if (m.GetKey()==ADD_CLIENT) {
        SocketType type = static_cast<SocketType>(m.GetIntValue());
        if (type!=CLIENT) SwitchClientType(mess.GetSocketId(), type);
      }
    }
    // Send the client's unique identifier
    Send(SocketMessage(SET_CLIENT_ID, mess.GetSocketId()), mess.GetSocketId());
    std::cout << "after sending the client id=" << mess.GetSocketId() << "!" << std::endl;
    SocketMessage(SET_CLIENT_ID, mess.GetSocketId()).Dump();
  } catch (Exception& e) {
    e.Dump();
  }
}

void
Messenger::SwitchClientType(int sid, SocketType type)
{
  SocketType oldtype;
  try {
    oldtype = GetSocketType(sid);
    fSocketsConnected.erase (std::pair<int,SocketType>(sid, oldtype));
    fSocketsConnected.insert(std::pair<int,SocketType>(sid, type));    
  } catch (Exception& e) {
    e.Dump();
  }
}

void
Messenger::ProcessMessage(SocketMessage m, int sid)
{
  if (m.GetKey()==REMOVE_CLIENT) {
    MessageKey key;
    
    if (m.GetIntValue()==GetSocketId()) {
      std::ostringstream o;
      o << "Some client (id=" << sid << ") asked for this master's disconnection!"
        << "\n\tIgnoring this request...";
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
      return;
    }
    
    if (sid==m.GetIntValue()) key = THIS_CLIENT_DELETED;
    else key = OTHER_CLIENT_DELETED;
    DisconnectClient(m.GetIntValue(), key);
  }
  else if (m.GetKey()==PING_CLIENT) {
    int toping = m.GetIntValue();
    Send(SocketMessage(PING_CLIENT), toping);
    SocketMessage msg; int i=0;
    do { msg = FetchMessage(toping); i++; } while (msg.GetKey()!=PING_ANSWER && i<2);
    Send(SocketMessage(PING_ANSWER, msg.GetValue()), sid);
  } 
  else if (m.GetKey()==GET_CLIENTS) {
    int i = 0;
    std::ostringstream os;
    for (SocketCollection::const_iterator it=fSocketsConnected.begin(); it!=fSocketsConnected.end(); it++, i++) {
      if (i!=0) os << ";";
      os << it->first << " (type " << static_cast<int>(it->second) << ")";
    }
    Send(SocketMessage(CLIENTS_LIST, os.str()), sid); 
  }
  else if (m.GetKey()==WEB_GET_CLIENTS) {
    int i = 0; SocketType type;
    std::ostringstream os;
    for (SocketCollection::const_iterator it=fSocketsConnected.begin(); it!=fSocketsConnected.end(); it++, i++) {
      if (it->first==GetSocketId()) type = MASTER; // master (us)
      else type = it->second;
      /*else {
        Send(SocketMessage(GET_CLIENT_TYPE), it->first);
        SocketMessage msg; int i=0;
        do { msg = FetchMessage(it->first); i++; } while (msg.GetKey()!=CLIENT_TYPE && i<2);
        type = static_cast<SocketType>(msg.GetIntValue());
      }*/
      
      if (i!=0) os << ";";
      
      os << it->first << ",";
      if (it->first==GetSocketId()) os << "Master,";
      else os << "Client" << it->first << ",";
      
      os << static_cast<int>(type);
    }
    try {
      Send(SocketMessage(CLIENTS_LIST, os.str()), sid);
    } catch (Exception& e) {
      e.Dump();
    }
  }
}

void
Messenger::Broadcast(const Message& m) const
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
