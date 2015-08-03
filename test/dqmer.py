#Socket client example in python
 
import sys  #for exit
sys.path.append("/home/ppstb/pps-tbrc/scripts")
import SocketHandler
import DQMReader

def dummy_computing(filename):
    reader = DQMReader.DQMReader(filename)
    reader.ProcessBinaryFile()
    resultfilename = reader.GetOutputFilename()
    return resultfilename

def run_dqm_process( messenger_socket, computation_process = dummy_computing ):
    while True: # while socket and connection are alive, but, as people say, it is hard to check
        incoming_message = ""
        result_filename = ""

        # RECEIVE
        incoming_message = messenger_socket.Receive(4096)

        # COMPUTE
        if len(incoming_message) != 2:
            print "Got wrong message:\n%s" % incoming_message # OR send error to the messenger?
            # MAYBE add 1 more message -- "KILL_DQM" -- and stop DQM process with it?
            continue
        if incoming_message[0] == "NEW_FILENAME":
            result_filename = computation_process(incoming_message[1])

        # SEND
        try:
            messenger_socket.Send("NEW_DQM_PLOT", result_filename)
        except SocketHandler.SendingError:
            #Send failed
            print 'Send failed'
            sys.exit()

if __name__ == '__main__':
    host = "localhost"
    socket_to_messenger = SocketHandler.SocketHandler( host, 1987 )
    socket_to_messenger.Handshake( 4 ) # 4 = DQM
    run_dqm_process( socket_to_messenger )
