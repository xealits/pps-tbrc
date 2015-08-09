#include "Client.h"
#include "Exception.h"
#include "OnlineDBHandler.h"

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
      inline DQMProcess(int port, unsigned short order=0, const char* det_type="") :
        Client(port), fOrder(order), fRunNumber(0), fDetectorType(det_type) {
        Client::Connect(Socket::DQM);
        SocketMessage run_msg = Client::SendAndReceive(GET_RUN_NUMBER, RUN_NUMBER);
        std::cout << "Current run number is: " << run_msg.GetIntValue() << std::endl;
        fRunNumber = run_msg.GetIntValue();
        IsInRun();
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
	    int ret = ParseMessage(&board_address, &filename);
            if (ret!=1) continue;
            // new raw file to process
            try { status = fcn(board_address, filename, &outputs); } catch (Exception& e) { Client::Send(e); continue; }
            if (!status) continue; // failed to produce plots
            cout << "Produced " << outputs.size() << " plot(s) for board with address 0x" << hex << board_address << endl;
            MessageKey key = INVALID_KEY;
            switch (act) {
              case NewPlot: key = NEW_DQM_PLOT; break;
              case UpdatedPlot: key = UPDATED_DQM_PLOT; break;
            }
            //sleep(fOrder);
            for (vector<string>::iterator nm=outputs.begin(); nm!=outputs.end(); nm++) {
              Client::Send(SocketMessage(key, *nm)); usleep(500);
            }
          }
        } catch (Exception& e) { /*Client::Send(e);*/ e.Dump(); }
      }
      /// Run a DQM plotter without any information on the board/output filename
      inline void Run(bool (*fcn)(vector<std::string>* outputs), const Action& act=NewPlot) {
        bool status = false;
        uint32_t board_address; std::string filename;
        std::vector<string> outputs;
        try {
          while (true) {
            outputs.clear();
            int ret =  ParseMessage(&board_address, &filename);
            if (ret!=1) continue;
            // new raw file to process
            try { status = fcn(&outputs); } catch (Exception& e) { Client::Send(e); continue; }
            if (!status) continue; // failed to produce plots
            cout << "Produced " << outputs.size() << " plot(s)" << endl;
            MessageKey key = INVALID_KEY;
            switch (act) {
              case NewPlot: key = NEW_DQM_PLOT; break;
              case UpdatedPlot: key = UPDATED_DQM_PLOT; break;
            }
            //sleep(fOrder);
            for (vector<string>::iterator nm=outputs.begin(); nm!=outputs.end(); nm++) {
              Client::Send(SocketMessage(key, *nm)); usleep(500);
            }
          } // end of infinite loop to fetch messages
        } catch (Exception& e) { Client::Send(e); e.Dump(); }
      }
    private:
      int ParseMessage(uint32_t* board_address, std::string* filename) {
        SocketMessage msg = Client::Receive(NEW_FILENAME);
        if (msg.GetKey()==NEW_FILENAME) {
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
          if (fDetectorType=="") return 1;
          if (fAddressesCanProcess.find(*board_address)==fAddressesCanProcess.end()) {
            std::cout << "board address " << *board_address << " is not in run" << std::endl;
            return 0;
          }
          
          std::cout << "Board address: " << *board_address << ", filename: " << *filename << std::endl;
          return 1;
        }
        else if (msg.GetKey()==RUN_NUMBER) {
          try { fRunNumber = msg.GetIntValue(); } catch (Exception& e) {
            std::cout << "Invalid Run number received: " << msg.GetValue() << std::endl;
            return -2;
          }
          if (IsInRun()) return 0;
          else return -3;
        }
        else {
          std::cout << "Invalid message received: " << MessageKeyToString(msg.GetKey()) << std::endl;
          return -1;
        }
        return -1;
      }
      inline bool IsInRun() {
        if (fDetectorType=="") return true; // no particular reason to leave it outside run
        fAddressesCanProcess.clear();
        std::string type = "";
        try {
          OnlineDBHandler::TDCConditionsCollection cc;
          try { cc = OnlineDBHandler().GetTDCConditions(fRunNumber); } catch (Exception& e) {
            e.Dump();
            std::cout << "Trying to fetch last run's conditions..." << std::endl;
            cc = OnlineDBHandler().GetTDCConditions(fRunNumber-1); //FIXME why is this happening????
          }
          for (OnlineDBHandler::TDCConditionsCollection::const_iterator c=cc.begin(); c!=cc.end(); c++) {
            if (c->detector.find(fDetectorType)!=std::string::npos) {
              std::cout << "Detectors of type \"" << fDetectorType << "\" are present in the run!"
                        << " Let's process them in a (near) future!" << std::endl;
              fAddressesCanProcess.insert(std::pair<unsigned long, std::string>(c->tdc_address, c->detector));
            }
          }
          if (fAddressesCanProcess.size()!=0) return true;
          std::cout << "Detector not in conditions... leaving this DQM (" << fDetectorType << ") hanging." << std::endl;
          return false;
        } catch (Exception& e) {
          e.Dump();
          std::cout << "Failed to retrieve online TDC conditions. Aborting" << std::endl;
          return false;
        }
      }
      unsigned short fOrder;
      unsigned int fRunNumber;
      std::string fDetectorType;
      std::map<unsigned long, std::string> fAddressesCanProcess;
  };
}
