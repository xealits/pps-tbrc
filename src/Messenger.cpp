#include "Messenger.h"

Messenger::Messenger() :
  Socket(-1), fWS(0), fNumAttempts(0), fPID(-1)
{}

Messenger::Messenger(int port) :
  Socket(port), fWS(0), fNumAttempts(0), fPID(-1)
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
Messenger::AddClient()
{
  Socket s;
  try {
    AcceptConnections(s);
    Message message = FetchMessage(s.GetSocketId());
    if (message.IsFromWeb()) {
      // Feed the handshake to the WebSocket object
      fWS->parseHandshake((unsigned char*)message.GetString().c_str(), message.GetString().size());
      Send(Message(fWS->answerHandshake()), s.GetSocketId());
      // From now on handle a WebSocket connection
      SwitchClientType(s.GetSocketId(), WEBSOCKET_CLIENT);
    }
    else {
      // if not a WebSocket connection
      SocketMessage m(message);
      if (m.GetKey()==ADD_CLIENT) {
        SocketType type = static_cast<SocketType>(m.GetIntValue());
        if (type!=CLIENT) SwitchClientType(s.GetSocketId(), type);
      }
    }
    // Send the client's unique identifier
    Send(SocketMessage(SET_CLIENT_ID, s.GetSocketId()), s.GetSocketId());
  } catch (Exception& e) {
    e.Dump();
  }
}

void
Messenger::DisconnectClient(int sid, MessageKey key, bool force)
{
  SocketType type;
  try { type = GetSocketType(sid); } catch (Exception& e) {
    e.Dump();
    return;
  }
  std::ostringstream o; o << "Disconnecting client # " << sid;
  if (type==WEBSOCKET_CLIENT) o << " (web socket)";
  PrintInfo(o.str());
  
  if (type==WEBSOCKET_CLIENT) {
    try {
      Send(SocketMessage(key, sid), sid);
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
    Socket s;
    s.SetSocketId(sid);
    s.Stop();
  }
  fSocketsConnected.erase(std::pair<int,SocketType>(sid, type));
  FD_CLR(sid, &fMaster);
}

void
Messenger::SwitchClientType(int sid, SocketType type)
{
  try {
    fSocketsConnected.erase (std::pair<int,SocketType>(sid, GetSocketType(sid)));
    fSocketsConnected.insert(std::pair<int,SocketType>(sid, type));    
  } catch (Exception& e) { e.Dump(); }
}

void
Messenger::Send(const Message& m, int sid) const
{
  try {
    Message tosend = (IsWebSocket(sid)) ? HTTPMessage(fWS, m, EncodeMessage) : m;
    //std::cout << "sending to " << sid << " --> web socket? " << IsWebSocket(sid) << std::endl;
    SendMessage(tosend, sid);
  } catch (Exception& e) { e.Dump(); }
}

void
Messenger::Receive()
{
  Message msg;
  SocketMessage m;
  
  // We start by copying the master file descriptors list to the
  // temporary list for readout
  fReadFds = fMaster;
  
  try { SelectConnections(); } catch (Exception& e) {
    e.Dump();
    throw Exception(__PRETTY_FUNCTION__, "Impossible to select the connections!", Fatal);
  }
  
  // Looking for something to read!
  for (SocketCollection::const_iterator s=fSocketsConnected.begin(); s!=fSocketsConnected.end(); s++) {
    if (!FD_ISSET(s->first, &fReadFds)) continue;
    
    // First check if we need to handle new connections
    if (s->first==GetSocketId()) { AddClient(); return; }
    
    // Handle data from a client
    try { msg = FetchMessage(s->first); } catch (Exception& e) {
      e.Dump();
      if (e.ErrorNumber()==11000) { DisconnectClient(s->first, THIS_CLIENT_DELETED); return; }
    }
    if (s->second==WEBSOCKET_CLIENT) {
      HTTPMessage h_msg(fWS, msg, DecodeMessage);
      try { m = SocketMessage(h_msg); } catch (Exception& e) {;}
    }
    else m = SocketMessage(msg.GetString());
    // Message was successfully decoded
    fNumAttempts = 0;
    
    try { ProcessMessage(m, s->first); } catch (Exception& e) {
      if (e.ErrorNumber()==11001) break;
    }
  }
}

void
Messenger::ProcessMessage(SocketMessage m, int sid)
{
  if (m.GetKey()==REMOVE_CLIENT) {
    if (m.GetIntValue()==GetSocketId()) {
      std::ostringstream o;
      o << "Some client (id=" << sid << ") asked for this master's disconnection!"
        << "\n\tIgnoring this request...";
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
      return;
    }
    const MessageKey key = (sid==m.GetIntValue()) ? THIS_CLIENT_DELETED : OTHER_CLIENT_DELETED;
    DisconnectClient(m.GetIntValue(), key);
    throw Exception(__PRETTY_FUNCTION__, "Removing socket client", Info, 11001);
  }
  else if (m.GetKey()==PING_CLIENT) {
    const int toping = m.GetIntValue();
    Send(SocketMessage(PING_CLIENT), toping);
    SocketMessage msg; int i=0;
    do { msg = FetchMessage(toping); i++; } while (msg.GetKey()!=PING_ANSWER && i<MAX_SOCKET_ATTEMPTS);
    try { Send(SocketMessage(PING_ANSWER, msg.GetValue()), sid); } catch (Exception& e) { e.Dump(); }
  }
  else if (m.GetKey()==GET_CLIENTS) {
    int i = 0; std::ostringstream os;
    for (SocketCollection::const_iterator it=fSocketsConnected.begin(); it!=fSocketsConnected.end(); it++, i++) {
      if (i!=0) os << ";";
      os << it->first << " (type " << static_cast<int>(it->second) << ")";
    }
    try { Send(SocketMessage(CLIENTS_LIST, os.str()), sid); } catch (Exception& e) { e.Dump(); }
  }
  else if (m.GetKey()==WEB_GET_CLIENTS) {
    int i = 0; SocketType type; std::ostringstream os;
    for (SocketCollection::const_iterator it=fSocketsConnected.begin(); it!=fSocketsConnected.end(); it++, i++) {
      type = (it->first==GetSocketId()) ? MASTER : it->second;
      if (i!=0) os << ";";
      os << it->first << ",";
      if (it->first==GetSocketId()) os << "Master,";
      else os << "Client" << it->first << ",";
      os << static_cast<int>(type) << "\0";
    }
    try { Send(SocketMessage(CLIENTS_LIST, os.str()), sid); } catch (Exception& e) { e.Dump(); }
  }
  else if (m.GetKey()==START_ACQUISITION) {
    try { StartAcquisition(); } catch (Exception& e) { e.Dump(); }
  }
  else if (m.GetKey()==STOP_ACQUISITION) {
    try { StopAcquisition(); } catch (Exception& e) { e.Dump(); }
  }
  else if (m.GetKey()==GET_RUN_NUMBER) {
    try {
      Send(SocketMessage(RUN_NUMBER, static_cast<int>(time(NULL))), sid);
    } catch (Exception& e) { e.Dump(); }
  }
  else if (m.GetKey()==SET_NEW_FILENAME) {
    try {
      std::cout << "---> " << m.GetValue() << std::endl;
      SendAll(DQM, SocketMessage(NEW_FILENAME, m.GetValue().c_str()));
    } catch (Exception& e) { e.Dump(); }
  }
  else if (m.GetKey()==NEW_DQM_PLOT) {
    try {
      m.Dump();
      SendAll(WEBSOCKET_CLIENT, m);
    } catch (Exception& e) { e.Dump(); }
  }
  else if (m.GetKey()==EXCEPTION) {
    try {
      SendAll(WEBSOCKET_CLIENT, m);
      Broadcast(m);
      std::cout << "------> " << m.GetValue() << std::endl;
    } catch (Exception& e) { e.Dump(); }
  }
  /*else {
    try { Send(SocketMessage(INVALID_KEY), sid); } catch (Exception& e) { e.Dump(); }
    std::ostringstream o;
    o << "Received an invalid message: " << m.GetString();
    throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
  }*/
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

void
Messenger::StartAcquisition()
{
  fPID = fork();

  try {
    switch (fPID) {
      case -1:
        throw Exception(__PRETTY_FUNCTION__, "Failed to fork the current process!", JustWarning);
      case 0:
        PrintInfo("Launching the daughter acquisition process");
        execl("ppsFetch", "", (char*)NULL);
        //close(fStdoutPipe[1]);
        throw Exception(__PRETTY_FUNCTION__, "Failed to launch the daughter process!", JustWarning);
      default:
        break;
    }
    SendAll(WEBSOCKET_CLIENT, SocketMessage(ACQUISITION_STARTED));
    throw Exception(__PRETTY_FUNCTION__, "Acquisition started!", Info, 30000);
  } catch (Exception& e) { e.Dump(); }
}

void
Messenger::StopAcquisition()
{
  signal(SIGCHLD, SIG_IGN);
  int ret = kill(fPID, SIGINT);
  if (ret<0) {
    std::ostringstream os;
    os << "Failed to kill the acquisition process with pid=" << fPID << "\n\t"
       << "Return value: " << ret << " (errno=" << errno << ")";
    throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
  }
  SendAll(WEBSOCKET_CLIENT, SocketMessage(ACQUISITION_STOPPED));
  throw Exception(__PRETTY_FUNCTION__, "Acquisition stopped!", Info, 30001);
}
