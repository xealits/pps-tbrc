#ifndef MessageKeys_h
#define MessageKeys_h

#include "messages.h"

/// This is where we define the list of available socket messages to be
/// sent/received by any actor.

MESSAGES_ENUM(\
  // generic messages
  INVALID_KEY, WEBSOCKET_KEY, EXCEPTION,\
  
  // client messages
  ADD_CLIENT, REMOVE_CLIENT, GET_CLIENTS, CLIENT_TYPE, PING_CLIENT,\
  GET_RUN_NUMBER, SET_NEW_FILENAME,\
  
  // master messages
  MASTER_BROADCAST, MASTER_DISCONNECT,\
  SET_CLIENT_ID,\
  THIS_CLIENT_DELETED, OTHER_CLIENT_DELETED,\
  CLIENTS_LIST, GET_CLIENT_TYPE, PING_ANSWER,\
  ACQUISITION_STARTED, ACQUISITION_STOPPED,\
  RUN_NUMBER, NEW_FILENAME,\
  
  // web socket messages
  WEB_GET_CLIENTS,\
  START_ACQUISITION, STOP_ACQUISITION,\

  // DQM messages
  NEW_DQM_PLOT, UPDATED_DQM_PLOT,\

  // other
  OTHER_MESSAGE,\
);

#endif

