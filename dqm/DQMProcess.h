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
      inline DQMProcess(int port, unsigned short order=0) : Client(port), fOrder(order) {
        Client::Connect(Socket::DQM);
      }
      inline ~DQMProcess() { Client::Disconnect(); }

      enum Action { NewPlot = 0x0, UpdatedPlot = 0x1 };

      /// Run a DQM plotter making use of the board/output filename information
      inline void Run(bool (*fcn)(unsigned int addr, std::string filename, vector<std::string>* outputs), const Action& act=NewPlot) {
        bool status = false;
        uint32_t board_address; std::string filename;
        std::vector<string> outputs;
        try {
          while (true) {
            outputs.clear();
            ParseMessage(&board_address, &filename);
            try { status = fcn(board_address, filename, &outputs); } catch (Exception& e) { Client::Send(e); continue; }
            if (status) {
              cout << "Produced " << outputs.size() << " plot(s) for board with address 0x" << hex << board_address << endl;
              MessageKey key;
              switch (act) {
                case NewPlot: key = NEW_DQM_PLOT; break;
                case UpdatedPlot: key = UPDATED_DQM_PLOT; break;
              }
              sleep(3*fOrder);
              for (vector<string>::iterator nm=outputs.begin(); nm!=outputs.end(); nm++) {
                Client::Send(SocketMessage(key, *nm)); sleep(1);
              }
            }
          }
        } catch (Exception& e) { Client::Send(e); e.Dump(); }
      }
      /// Run a DQM plotter without any information on the board/output filename
      inline void Run(bool (*fcn)(vector<std::string>* outputs), const Action& act=NewPlot) {
        bool status = false;
        uint32_t board_address; std::string filename;
        std::vector<string> outputs;
        try {
          while (true) {
            outputs.clear();
            ParseMessage(&board_address, &filename);
            try { status = fcn(&outputs); } catch (Exception& e) { Client::Send(e); continue; }
            if (status) {
              cout << "Produced " << outputs.size() << " plot(s)" << endl;
              MessageKey key;
              switch (act) {
                case NewPlot: key = NEW_DQM_PLOT; break;
                case UpdatedPlot: key = UPDATED_DQM_PLOT; break;
              }
              sleep(3*fOrder);
              for (vector<string>::iterator nm=outputs.begin(); nm!=outputs.end(); nm++) {
                Client::Send(SocketMessage(key, *nm)); sleep(1);
              }
            }
          }
        } catch (Exception& e) { Client::Send(e); e.Dump(); }
      }
    private:
      void ParseMessage(uint32_t* board_address, std::string* filename) {
        SocketMessage msg = Client::Receive(NEW_FILENAME); if (msg.GetKey()!=NEW_FILENAME) { return; }
        if (msg.GetValue()=="") {
          std::ostringstream os; os << "Invalid output file path received through the NEW_FILENAME message: " << msg.GetValue();
          throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning);
        }
        std::string value = msg.GetValue();
        size_t end = value.find(':');
        if (end==std::string::npos) {
          std::ostringstream s; s << "Invalid filename message built! (\"" << value << "\")";
          throw Exception(__PRETTY_FUNCTION__, s.str().c_str(), JustWarning);
        }
        *board_address = atoi(value.substr(0, end).c_str());
        *filename = value.substr(end+1);
      }
      unsigned short fOrder;
  };
}
