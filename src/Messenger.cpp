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

  // return a true iff everything went fine !
  return true;
}

void
Messenger::Disconnect()
{
  if (fPort<0) return; // do not broadcast the death of a secondary messenger!
  //if (fListeners.size()) {
    /*for (SocketRefCollection::iterator s=fSocketsConnected.begin(); s!=fSocketsConnected.end(); s++) {
      static_cast<Messenger*>(*s)->Disconnect();
    }*/
    try {
      Broadcast(Message(MASTER_DISCONNECT, ""));
    } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to broadcast the server disconnection status!", JustWarning, SOCKET_ERROR(errno));
    }
  //}
  Stop();
}

/*MessageKey
Messenger::Receive()
{
  MessageKey key = INVALID_KEY;
  std::ostringstream s;
  try {
    Message message = FetchMessage(false);
    
    if (message.String().empty()) {
      std::cout << "---> EMPTY KEY" << std::endl;
      return INVALID_KEY;
    }
    std::cout << "---> NOT EMPTY KEY" << std::endl;    
        
    // We determine the message type
    key = message.GetKey();
    
    std::cout << __PRETTY_FUNCTION__ << " received \"" << MessageKeyToString(message.GetKey()) << "\" -> \"" << message.GetValue() << "\"" << std::endl;
    
    switch(key) {
    case ADD_LISTENER:
      std::cout << "New client ! -> " << fLastListenerAdded << std::endl;
      
      if (fLastListenerAdded<0) fLastListenerAdded = 0; // first listener added!
      else fLastListenerAdded += 1;
      
      fListeners.insert(fLastListenerAdded);
      SendMessage(Message(SET_LISTENER_ID, fLastListenerAdded));
      break;
      
    case REMOVE_LISTENER:
      std::cout << "Delete client !" << std::endl;
      if (fListeners.erase(message.GetIntValue())<=0) {
        std::ostringstream os;
        os << "Warning: Risk of listeners list corruption!" << std::endl
           << "\tTried to remove a non-registered listener.";
        throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
      }
      s.str(""); s << "Client " << message.GetValue() << " is to be deleted";
      try {
        SendMessage(Message(LISTENER_DELETED, message.GetValue()));
      } catch (Exception& e) {
        e.Dump();
      }
      //throw Exception(__PRETTY_FUNCTION__, s.str(), Info);
      break;
      
    case INVALID_KEY:
    default:
      std::cout << "------> " << message.GetValue() << std::endl;
      throw Exception(__PRETTY_FUNCTION__, "Received an invalid message!", JustWarning);
    }
    DumpListeners();
  } catch (Exception& e) {
    if (e.Type()>Info) {
      e.Dump();
    }
    throw Exception(__PRETTY_FUNCTION__, "Message with invalid key received!", Info);
    return key;
  }
  return key;
}*/

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
  //for (int i=0; i<=*fSocketsConnected.rbegin(); i++) {
    
    if (!FD_ISSET(*sid, &fReadFds)) continue;
    
    // First check if we need to handle new connections
    if (*sid==GetSocketId()) {
      Messenger mess;
      try {
        AcceptConnections(mess);
        SendMessage(Message(SET_LISTENER_ID, *sid), *sid);
      } catch (Exception& e) {
        e.Dump();
      }
      std::cout << "new connection at " << mess.GetSocketId() << "!" << std::endl;
      //DumpConnected();
      return MessageKey();
    }
    
    // Handle data from a client
    std::cout << "new message from " << *sid << std::endl;
    Message message = FetchMessage(*sid);
    message.Dump();
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
    //case 
  }
}

void
Messenger::Broadcast(Message m)
{
  try {
    SendMessage(m, -1);
  } catch (Exception& e) {
    e.Dump();
  }
}
