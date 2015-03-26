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

bool
Messenger::Receive()
{
  try {
    std::string message = FetchMessage();
    if (!message.empty()) {
      std::cout << __PRETTY_FUNCTION__ << " received \"" << message << "\"" << std::endl;
    }
    
    SendMessage("haha");
  } catch (Exception& e) {
    e.Dump();
    return false;
  }
  
  return true;
}

