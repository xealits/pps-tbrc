#ifndef MessageKeys_h
#define MessageKeys_h

#include "messages.h"

/// This is where we define the list of available socket messages to be
/// sent/received by any actor.

MESSAGES_ENUM(\
  // generic messages
  INVALID_KEY, WEBSOCKET_KEY,                                          \
  
  // listener messages
  ADD_LISTENER, REMOVE_LISTENER, GET_LISTENERS,                        \
  
  // master messages
  MASTER_BROADCAST, MASTER_DISCONNECT,                                 \
  SET_LISTENER_ID, LISTENER_DELETED,                                   \
  LISTENERS_LIST,                                                      \
  
  // web socket messages
  WEB_GET_LISTENERS,                                                   \
);  

#endif

