#include "Messenger.h"

Messenger::Messenger(int port) :
  Socket(port), fLastListenerAdded(-1)
{
  std::cout << __PRETTY_FUNCTION__ << " new Messenger at port " << GetPort() << std::endl;
  std::cout << "!!!!!!!!!!!!!!! " << fLastListenerAdded << std::endl;
}

Messenger::~Messenger()
{}

bool
Messenger::Connect()
{
  try {
    Start();
    Bind();
    Listen(5);
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
  if (fListeners.size()) {
    try {
      Broadcast("disconnect_server");
    } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Failed to broadcast the server disconnection status!", JustWarning, SOCKET_ERROR(errno));
    }
  }
}

MessageKey
Messenger::Receive()
{
  MessageKey key = INVALID_KEY;
  try {
    Message message = FetchMessage();
    message.Dump();
    
    if (message.String().empty()) return INVALID_KEY;
    
    //std::cout << __PRETTY_FUNCTION__ << " received \"" << message.GetKey() << "\" -> \"" << message.GetValue() << "\"" << std::endl;
    
    // We determine the message type
    key = message.GetKey();
    
    if (key==ADD_LISTENER) {
      std::cout << "New client !" << std::endl;
      
      std::cout << "--> " << fLastListenerAdded << " added" << std::endl;
      if (fLastListenerAdded<0) fLastListenerAdded = 0; // first listener added!
      else fLastListenerAdded += 1;
      
      std::cout << "--> " << fLastListenerAdded << " added" << std::endl;
      fListeners.push_back(fLastListenerAdded);
      //SendMessage(Message(SET_LISTENER_ID, fLastListenerAdded));
      //std::cout << MessageKeyToString(SET_LISTENER_ID) << std::endl;
      SendMessage(Message(SET_LISTENER_ID, "huhu"));
    }
    if (key==REMOVE_LISTENER) {
      std::cout << "Delete client !" << std::endl;
      //SendMessage(Message("huhu", "aaah"));
    }
    //else if (message=="")
    
  } catch (Exception& e) {
    throw Exception(__PRETTY_FUNCTION__, "Message with invalid key received!", JustWarning, SOCKET_ERROR(errno));
    e.Dump();
    return INVALID_KEY;
  }
  
  return key;
}

void
Messenger::Broadcast(std::string message)
{
  try {
    SendMessage(Message(MASTER_BROADCAST, message.c_str()));
  } catch (Exception& e) {
    e.Dump();
  }
}
