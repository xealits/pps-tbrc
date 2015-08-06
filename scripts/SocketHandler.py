#!/usr/bin/env python
import asyncore
import socket
import sys
import os, re
from cStringIO import StringIO

class AsyncClient(asyncore.dispatcher):
  def __init__(self, host, port):
    asyncore.dispatcher.__init__(self)
    self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
    address = (host, port)
    self.connect(address)
    self.write_buffer = ''
    self.read_buffer = StringIO()

  def handle_connect(self):
    print 'handle_connect()'

  def handle_close(self):
    self.close()

  def readable(self):
    return True

  def writable(self):
    return (len(self.write_buffer)>0)
    
  def handle_write(self):
    print 'handle_write()'
    sent = self.send(self.write_buffer)
    self.write_buffer = self.write_buffer[sent:]

  def handle_read(self):
    #print 'handle_read()'
    data = self.recv(8192)
    if data:
      self.read_buffer.write(data)
      raise asyncore.ExitNow('Message fetched!')

  def fileno(self):
    return self.socket.fileno()

  def serve_until_message(self):
    try:
      asyncore.loop(1, 5)
    except asyncore.ExitNow, e:
      #_split = re.compile(r'[\0%s]' % re.escape(''.join([os.path.sep, os.path.altsep or ''])))
      msgs = []
      for msg in self.read_buffer.getvalue().split('\0'):
        out = msg.split(':')
        msgs.append((out[0], ':'.join(out[1:]).replace('\0', '')))
      self.read_buffer = StringIO()
      #return (out[0], _split.sub('', ':'.join(out[1:])))
      return msgs

class SocketHandler:
  class SocketError(Exception):
    pass
  class SendingError(Exception):
    pass
  class InvalidMessage(Exception):
    pass

  def __init__(self, host, port):
    self.client_id = -1
    try:
      self.socket = AsyncClient(host, port)
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

  def fileno(self):
    return self.socket.fileno()

  def Handshake(self, client_type):
    try:
      self.Send('ADD_CLIENT', client_type)
    except SendingError:
      print "Impossible to send client type. Exiting"
      raise self.SocketError
      sys.exit()
    msg = self.Receive()
    print msg
    if msg and msg[0]=='SET_CLIENT_ID':
      self.client_id = msg[1]
      print 'Client id='+ self.client_id

  def GetClientId(self): return self.client_id

  def Disconnect(self):
    try:
      self.Send('REMOVE_CLIENT', self.client_id)
    except SendingError:
      print "Failed to disconnect the GUI from master. Exiting roughly..."
      sys.exit()

  def ReceiveAll(self):
    return self.socket.serve_until_message()

  def Receive(self, key=None, max_num_trials=2):
    num_trials = 0
    while True:
      num_trials += 1
      out = self.socket.serve_until_message()
      if not out: continue
      if not key: return out[0] #FIXME what about the others?
      print out
      for msg in out:
        if len(msg)>1 and msg[0]==key: return msg
      if num_trials>max_num_trials: return None

  def Send(self, key, value):
    try:
      self.socket.sendall(key+':'+str(value))
    except socket.error:
      raise self.SendingError

