#ifndef RunFile_h
#define RunFile_h

#include <string>
#include <map>
#include <fstream>

/**
 * \brief Handler for the run information file reader
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 3 Aug 2015
 */
class RunFileHandler
{
  public:
    inline RunFileHandler(std::string path="./run_info.dat") {
      std::ifstream test(path.c_str());
      if (test.good()) { test.close(); }
      else throw Exception(__PRETTY_FUNCTION__, "File does not exist! Aborting.", JustWarning);
      fFile.open(path.c_str());
    }
    inline ~RunFileHandler() { fFile.close(); }
    
    /// Retrieve the last run acquired
    unsigned int GetLastRun() {
      if (!fFile.is_open()) throw Exception(__PRETTY_FUNCTION__, "File not successfully loaded. Aborting.", JustWarning);
      unsigned int run_id, spill_id, timestamp;
      unsigned int last_run_id = 0;
      while (fFile >> run_id >> spill_id >> timestamp) {
        if (run_id>last_run_id) last_run_id = run_id;
      }
      fFile.clear();
      fFile.seekg(0, ios::beg);
      return last_run_id;
    }

    typedef std::map<unsigned int, unsigned int> SpillInfos;
    /// Retrieve information on a given run (spill IDs / timestamp)
    SpillInfos GetRunInfo(unsigned int run) {
      if (!fFile.is_open()) throw Exception(__PRETTY_FUNCTION__, "File not successfully loaded. Aborting.", JustWarning);
      unsigned int run_id, spill_id, timestamp;
      SpillInfos out;
      while (fFile >> run_id >> spill_id >> timestamp) {
        if (run_id==run) out.insert(std::pair<unsigned int, unsigned int>(spill_id, timestamp));
      }
      fFile.clear();
      fFile.seekg(0, ios::beg);
      return out;
    }

    
  private:
    std::ifstream fFile;
};

#endif
