#!/usr/bin/env python

import socket
import sys
import os, re

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
    try:
      self.socket.connect(infos)
    except socket.error:
      raise self.SocketError

  def Handshake(self, client_type):
    try:
      self.Send('ADD_CLIENT', client_type)
    except SendingError:
      print "Impossible to send client type. Exiting"
      raise self.SocketError
      sys.exit()
    try:
      msg = self.Receive()
    except ReceivingError:
      print "Socket receiving error during handshake. Exiting"
      sys.exit()
    except InvalidMessage:
      print "Invalid message received in handshake. Exiting"
      sys.exit()
    if msg[0]=='SET_CLIENT_ID':
      self.client_id = msg[1]
      print 'Client id='+ self.client_id

  def GetClientId(self): return self.client_id

  def Disconnect(self):
    try:
      self.Send('REMOVE_CLIENT', self.client_id)
    except SendingError:
      print "Failed to disconnect the GUI from master. Exiting roughly..."
      sys.exit()

  def Receive(self, key=None):
    num_trials = 0
    while True:
      num_trials += 1
      try:
        recv = self.socket.recv(4096)
      except socket.error:
        raise self.ReceivingError
      if ':' not in recv:
        raise self.InvalidMessage
      out = recv.split(':')
      if key==None or out[0]==key: break
      if num_trials>2: return None
      
    _split = re.compile(r'[\0%s]' % re.escape(''.join([os.path.sep, os.path.altsep or ''])))
    return (out[0], _split.sub('', ':'.join(out[1:])))

  def Send(self, key, value):
    try:
      self.socket.sendall(key+':'+str(value))
    except socket.error:
      raise self.SendingError

