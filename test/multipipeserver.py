#!/usr/bin/env python

import socket
import sys

import thread
import time
 
HOST = 'localhost'   # Symbolic name meaning all available interfaces
PORT = 1982 # Arbitrary non-privileged port
 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print 'Socket created'
 
try:
    s.bind((HOST, PORT))
except socket.error , msg:
    print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
    sys.exit()
     
print 'Socket bind complete'
 
s.listen(3)
print 'Socket now listening'

connections = []

#Function for handling connections. This will be used to create threads
def clientthread(conn):
    #Sending message to connected client
    conn.send('Welcome to the server. Type something and hit enter\n') #send only takes string
     
    #infinite loop so that function do not terminate and thread do not end.
    while True:
         
        #Receiving from client
        data = conn.recv(1024)
        reply = data #'Simon says...' + data
        if not data: 
            break
     
        # conn.sendall(reply)
        for c in connections:
            c.sendall(reply)
     
    #came out of loop
    conn.close()


# def foo():
# 	while 1:
# 		yield 5
# 		yield "asdkams"
# 	pass


conn, addr = s.accept()
print 'Head connected with ' + addr[0] + ':' + str(addr[1])
thread.start_new_thread(clientthread ,(conn,))

#now keep talking with the client
while 1:
    #wait to accept a connection - blocking call
    time.sleep(5) # sleep 5 seconds
    conn, addr = s.accept()
    connections.append(conn)
    print 'Connected with ' + addr[0] + ':' + str(addr[1])
     
    #start new thread takes 1st argument as a function name to be run, second is the tuple of arguments to the function.
 
s.close()

