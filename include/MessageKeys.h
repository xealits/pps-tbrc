#ifndef MessageKeys_h
#define MessageKeys_h

#include "messages.h"

ENUM(\
  // generic messages
  INVALID_KEY,                                                                 \
  
  // listener messages
  ADD_LISTENER, REMOVE_LISTENER, SET_LISTENER_ID,                              \
  
  // master messages
  MASTER_BROADCAST                                                             \
);  

#endif

