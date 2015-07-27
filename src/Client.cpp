#include "Client.h"

Client::Client(int port) :
  Socket(port), fClientId(-1), fIsConnected(false)
{}

Client::~Client()
{
  if (fIsConnected) Disconnect();
}

bool
Client::Connect(const SocketType& type)
{
  fType = type;
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
  
void
Client::Announce()
{
  try {
    // Once connected we send our request for connection
    SendMessage(SocketMessage(ADD_CLIENT, static_cast<int>(GetType())));
    
    // Then we wait for to the server to send us a connection acknowledgement
    // + an id
    SocketMessage ack(FetchMessage());
    
    switch (ack.GetKey()) {
      case SET_CLIENT_ID:
        fClientId = ack.GetIntValue();
        break;
      case INVALID_KEY:
      default:
        throw Exception(__PRETTY_FUNCTION__, "Received an invalid answer from server", JustWarning);
    }
    
  } catch (Exception& e) {
    e.Dump();
  }
  
  std::cout << __PRETTY_FUNCTION__ << " connected to socket at port " << GetPort() << ", received id \"" << fClientId << "\""<< std::endl;
}

void
Client::Disconnect()
{
  std::cout << "===> Disconnecting the client from socket" << std::endl;
  if (!fIsConnected) return;
  try {
    SendMessage(SocketMessage(REMOVE_CLIENT, fClientId), -1);
  } catch (Exception& e) {
    e.Dump();
  }
  try {
    SocketMessage ack(FetchMessage());
    if (ack.GetKey()==THIS_CLIENT_DELETED or ack.GetKey()==OTHER_CLIENT_DELETED) {
      fIsConnected = false;
    }
  } catch (Exception& e) {
    if (e.ErrorNumber()!=11000) // client has been disconnected
      e.Dump();
  }
}

void
Client::Receive()
{
  SocketMessage msg;
  try {
    msg = FetchMessage();
  } catch (Exception& e) {
    if (e.ErrorNumber()==11000) // client has been disconnected
      throw Exception(__PRETTY_FUNCTION__, "Some other socket asked for this client's disconnection. Obtemperating...", Fatal);
    else e.Dump();
  }
  if (msg.GetKey()==MASTER_DISCONNECT) {
    throw Exception(__PRETTY_FUNCTION__, "Master disconnected!", Fatal);
  }
  else if (msg.GetKey()==OTHER_CLIENT_DELETED) {
    throw Exception(__PRETTY_FUNCTION__, "Some other socket asked for this client's disconnection. Obtemperating...", Fatal);
  }
  else if (msg.GetKey()==GET_CLIENT_TYPE) {
    Send(SocketMessage(CLIENT_TYPE, static_cast<int>(GetType())));
  } 
  else if (msg.GetKey()==PING_CLIENT) {
    ostringstream os; os << "Pong. My name is " << GetSocketId() << " and I feel fine, thank you!";
    Send(SocketMessage(PING_ANSWER, os.str()));
    PrintInfo("Got a ping, answering...");
  } 
  else if (msg.GetKey()==CLIENTS_LIST) {
    VectorValue vals = msg.GetVectorValue();
    int i = 0; std::ostringstream o; o << "List of members on the socket:\n\t";
    for (VectorValue::const_iterator v=vals.begin(); v!=vals.end(); v++, i++) {
      if (i!=0) o << ", ";
      o << *v;
    }
    PrintInfo(o.str());
  }
  else {
    ParseMessage(msg);
  }
}

SocketMessage
Client::Receive(const MessageKey& key)
{
  SocketMessage msg;
  try {
    msg = FetchMessage();
    if (msg.GetKey()==key) {
      return msg;
    }
  } catch (Exception& e) {
    e.Dump();
  }
  return msg;
}
