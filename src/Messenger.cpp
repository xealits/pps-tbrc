#include "Messenger.h"

Messenger::Messenger(int port) :
  Socket(port)
{
  std::cout << __PRETTY_FUNCTION__ << " new Messenger at port " << GetPort() << std::endl;
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

message_t
Messenger::Receive()
{
  message_t message_type = INVALID;
  try {
    Message message = FetchMessage();
    if (message.ToString().empty()) return INVALID;
    
    //std::cout << __PRETTY_FUNCTION__ << " received \"" << message.GetKey() << "\" -> \"" << message.GetValue() << "\"" << std::endl;
    
    // We determine the message type
    if (message.GetKey()=="new_client") {
      message_type = NEW_LISTENER;
      std::cout << "New client !" << std::endl;
      SendMessage(Message("huhu", "aaah"));
    }
    if (message.GetKey()=="terminate_client") {
      message_type = DEL_LISTENER;
      std::cout << "Delete client !" << std::endl;
      //SendMessage(Message("huhu", "aaah"));
    }
    //else if (message=="")
    
  } catch (Exception& e) {
    e.Dump();
    return INVALID;
  }
  
  return message_type;
}

void
Messenger::Broadcast(std::string message)
{
  try {
    SendMessage(Message("info", message.c_str()));
  } catch (Exception& e) {
    e.Dump();
  }
}
