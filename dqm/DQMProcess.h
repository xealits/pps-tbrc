#include "Client.h"
#include "Exception.h"

#define DQM_OUTPUT_DIR "/tmp/"

namespace DQM
{
  /**
   * \brief Handler for a common DQM process to run on the socket
   * \author Laurent Forthomme <laurent.forthomme@cern.ch>
   * \date 3 Aug 2015
   */
  class DQMProcess : public Client
  {
    public:
      inline DQMProcess(int port) : Client(port) {
        Client::Connect(Socket::DQM);
      }
      inline ~DQMProcess() { Client::Disconnect(); }

      inline void Run(bool (*fcn)(unsigned int addr, std::string filename, vector<std::string>* outputs)) {
        bool status = false;
        try {
          SocketMessage msg;
          while (true) {
            msg = Client::Receive(NEW_FILENAME); if (msg.GetKey()!=NEW_FILENAME) { continue; }
            if (msg.GetValue()=="") {
              std::ostringstream os; os << "Invalid output file path received through the NEW_FILENAME message: " << msg.GetValue();
              throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
            }
            std::vector<string> outputs; string value = msg.GetValue();
      
            size_t end; uint32_t board_address; string filename;
            if ((end=value.find(':'))==std::string::npos) {
              std::ostringstream s; s << "Invalid filename message built! (\"" << value << "\")";
              throw Exception(__PRETTY_FUNCTION__, s.str().c_str(), JustWarning);
            }
            board_address = atoi(value.substr(0, end).c_str());
            filename = value.substr(end+1);
            try {
              status = fcn(board_address, filename, &outputs);
            } catch (Exception& e) { Client::Send(e); continue; }
            if (status) {
              cout << "Produced " << outputs.size() << " plot(s) for board with address 0x" << hex << board_address << endl;
              for (vector<string>::iterator nm=outputs.begin(); nm!=outputs.end(); nm++) {
                sleep(1); Client::Send(SocketMessage(NEW_DQM_PLOT, *nm));
              }
            }
          }
        } catch (Exception& e) { e.Dump(); Client::Send(e); }
      }
  };
}
