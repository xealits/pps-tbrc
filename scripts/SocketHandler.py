#!/usr/bin/env python

import socket
import sys

class SocketHandler:
  class SocketError(Exception):
    pass
  class SendingError(Exception):
    pass
  class InvalidMessage(Exception):
    pass
  
  def __init__(self, host, port):
    try:
      self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except socket.error:
      print "Failed to create socket!"
      sys.exit()
    print "Socket created"

    try:
      remote_ip = socket.gethostbyname(host)
    except socket.gaierror:
      print "Hostname could not be resolved! Exiting"
      sys.exit()

    # connect to remote server
    infos = (remote_ip, port)
    self.socket.connect(infos)

  def Handshake(self, client_type):
    try:
      self.Send('ADD_CLIENT', client_type)
    except SocketHandler.SendingError:
      print "Impossible to send client type. Exiting"
      sys.exit()
    try:
      msg = self.Receive(4096)
    except ReceivingError:
      print "Socket receiving error during handshake. Exiting"
      sys.exit()
    except InvalidMessage:
      print "Invalid message received in handshake. Exiting"
      sys.exit()
    if msg[0]=='SET_CLIENT_ID':
      self.client_id = msg[1]
      print 'Client id='+ self.client_id

  def Receive(self, size):
    try:
      recv = self.socket.recv(size)
    except socket.error:
      raise ReceivingError
    if ':' not in recv:
      raise InvalidMessage
    out = recv.split(':')
    return (out[0], ':'.join(out[1:]))

  def Send(self, key, value):
    try:
      self.socket.sendall(key+':'+str(value))
    except socket.error:
      raise SendingError
