#Socket client example in python
 
import socket   #for sockets
import sys  #for exit


def make_client( host, port ):

    #create an INET, STREAMing socket
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except socket.error:
        print 'Failed to create socket'
        sys.exit()

    print 'Socket Created'

    try:
        remote_ip = socket.gethostbyname( host )
    except socket.gaierror:
        #could not resolve
        print 'Hostname could not be resolved. Exiting'
        sys.exit()

    #Connect to remote server
    s.connect((remote_ip , port))
    return s


def make_server( host, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print 'Socket created'

    try:
        s.bind((host, port))
    except socket.error , msg:
        print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        sys.exit()

    print 'Socket bind complete'

    s.listen(3)
    print 'Socket now listening'
    conn, addr = s.accept()
    print 'Got a listener'
    return conn


def dummy_computing(filename):
    print "Computed the %s" % filename
    return filename

def run_dqm_process( incomming_socket, listener_connection, computation = dummy_computing ):
    while True: # while socket and connection are alive, but, as people say, it is hard to check
        incomming_message = ""
        outgoing_message = ""
        incomming_message = incomming_socket.recv(4096)
        tokens = incomming_message.split(":")
        if len(tokens) == 2 and tokens[0] == "DQM":
            outgoing_message = computation( tokens[1] )
        else:
            print "Got wrong message:\n%s" % incomming_message
            continue
        listener_connection.sendall( outgoing_message )


# print 'Socket Connected to ' + host + ' on ip ' + remote_ip
 
# #Send some data to remote server
# message = "GET / HTTP/1.1\r\n\r\n"
 
# try :
#     #Set the whole string
#     s.sendall(message)
# except socket.error:
#     #Send failed
#     print 'Send failed'
#     sys.exit()
 
# print 'Message send successfully'
 
# #Now receive data
# reply = s.recv(4096)
 
# print reply


if __name__ == '__main__':
    host = "localhost"
    incomming_socket = make_client( host, 1982 )
    outgoing_connection = make_server( host, 8762 )
    run_dqm_process( incomming_socket, outgoing_connection )

