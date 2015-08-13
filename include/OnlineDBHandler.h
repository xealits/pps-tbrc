#ifndef OnlineDBHandler_h
#define OnlineDBHandler_h

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sqlite3.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "Exception.h"

static int callback(void* unused, int argc, char* argv[], char* azcolname[])
{
  for (int i=0; i<argc; i++) {
    std::cerr << azcolname[i] << " = " << (argv[i] ? argv[i] : "NULL") << std::endl;
  }
  return 0;
}

/**
 * \brief Handler for the run information online database
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 3 Aug 2015
 */
class OnlineDBHandler
{
  public:
    inline OnlineDBHandler(std::string path=std::string(std::getenv("PPS_PATH"))+"/run_infos.db") {
      int rc;
      bool build_tables;
      std::ifstream test(path.c_str());
      if (test.good()) { test.close(); build_tables = false; }
      else build_tables = true;

      rc = sqlite3_open(path.c_str(), &fDB);
      std::ostringstream os;
      if (rc) {
        os << "Cannot open online database file: " << sqlite3_errmsg(fDB);
        throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, 60000);
      }
      if (build_tables) BuildTables();
    }

    inline ~OnlineDBHandler() { sqlite3_close(fDB); }

    inline void NewRun() {
      char* err = 0;
      std::string req = "INSERT INTO run (id) VALUES (NULL)";
      int rc = sqlite3_exec(fDB, req.c_str(), callback, 0, &err);
      if (rc!=SQLITE_OK) {
        std::ostringstream os;
        os << "Error while trying to add a new run" << "\n\t"
           << "SQLite error: " << err;
        throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, 60100);
      }
    }
    
    inline void NewBurst() {
      char* err = 0;
      std::ostringstream req;
      unsigned int current_run = GetLastRun();
      int last_burst = 0;
      try { last_burst = GetLastBurst(current_run); } catch (Exception& e) {
        if (e.ErrorNumber()==60103) last_burst = -1;
      }
      req << "INSERT INTO burst (id, run_id, burst_id) VALUES (NULL, "
          << current_run << ", "
          << last_burst+1 << ");";
      int rc = sqlite3_exec(fDB, req.str().c_str(), callback, 0, &err);
      if (rc!=SQLITE_OK) {
        std::ostringstream os;
        os << "Error while trying to add a new burst" << "\n\t"
           << "SQLite error: " << err;
        throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, 60101);
      }
    }

    typedef std::map<unsigned int, unsigned int> RunCollection;
    inline RunCollection GetRuns() const {
      std::vector< std::vector<int> > out = Select<int>("SELECT id,start FROM run ORDER BY id");
      if (out.size()==0) {
        throw Exception(__PRETTY_FUNCTION__, "Trying to read runs in an empty database", JustWarning, 60200);
      }
      RunCollection ret;
      for (std::vector< std::vector<int> >::iterator it=out.begin(); it!=out.end(); it++) {
        ret.insert(std::pair<unsigned int, unsigned int>(it->at(0), it->at(1)));
      }
      return ret;
    }

    /// Retrieve the last run acquired
    inline unsigned int GetLastRun() const {
      std::vector< std::vector<int> > out = Select<int>("SELECT id FROM run ORDER BY id DESC LIMIT 1");
      if (out.size()==1) return out[0][0];
      if (out.size()==0) {
        throw Exception(__PRETTY_FUNCTION__, "Trying to read the last run of an empty database", JustWarning, 60200);
      }
      if (out.size()>1) {
        throw Exception(__PRETTY_FUNCTION__, "Corrupted database", JustWarning, 60010);
      }
      return 0;
    }

    inline int GetLastBurst(unsigned int run) const {
      std::ostringstream os;
      os << "SELECT burst_id FROM burst WHERE run_id=" << run << " ORDER BY burst_id DESC LIMIT 1";
      std::vector< std::vector<int> > out = Select<int>(os.str());
      if (out.size()==0) {
        std::ostringstream os;
        os << "Trying to read the last burst of a non-existant run: " << run;
        throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, 60103);
      }
      else if (out.size()==1) return out[0][0];
      return -1;
    }

    struct BurstInfo {
      unsigned int burst_id;
      unsigned int time_start;
    };
    typedef std::vector<BurstInfo> BurstInfos;
    /// Retrieve information on a given run (spill IDs / timestamp)
    inline BurstInfos GetRunInfo(unsigned int run) const {
      std::ostringstream os;
      BurstInfos ret; ret.clear();
      os << "SELECT burst_id,start FROM burst WHERE run_id=" << run << " ORDER BY burst_id";
      std::vector< std::vector<int> > out = Select<int>(os.str());
      for (std::vector< std::vector<int> >::iterator it=out.begin(); it!=out.end(); it++) {
        BurstInfo bi;
        bi.burst_id = it->at(0);
        bi.time_start = it->at(1);
        ret.push_back(bi);
      }
      return ret;
    }
    
    inline void SetTDCConditions(unsigned short tdc_id, unsigned long tdc_address, unsigned short tdc_acq_mode, unsigned short tdc_det_mode, std::string detector) {
      char* err = 0;
      std::ostringstream req;
      unsigned int current_run = GetLastRun();
      req << "INSERT INTO tdc_conditions (id, run_id, tdc_id, tdc_address, tdc_acq_mode, tdc_det_mode, detector) VALUES (NULL, "
          << current_run << ", "
          << tdc_id << ", "
          << tdc_address << ", "
          << tdc_acq_mode << ", "
          << tdc_det_mode << ", \""
          << detector << "\");";
      int rc = sqlite3_exec(fDB, req.str().c_str(), callback, 0, &err);
      if (rc!=SQLITE_OK) {
        std::ostringstream os;
        os << "Error while trying to add a new TDC condition" << "\n\t"
           << "SQLite error: " << err;
        throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, 60110);
      }
    }
    
    struct TDCConditions {
      unsigned int run_id;
      unsigned short tdc_id; unsigned long tdc_address;
      unsigned short tdc_acq_mode; unsigned short tdc_det_mode;
      std::string detector;
      bool operator==(const TDCConditions& rhs) const {
        return (tdc_id==rhs.tdc_id
            and tdc_address==rhs.tdc_address
            and tdc_acq_mode==rhs.tdc_acq_mode
            and tdc_det_mode==rhs.tdc_det_mode
            and detector.compare(rhs.detector)==0); // we leave the run id out of the comparison operator
      }
      TDCConditions& operator=(const TDCConditions& rhs) {
        run_id = rhs.run_id; tdc_id = rhs.tdc_id; tdc_address = rhs.tdc_address;
        tdc_acq_mode = rhs.tdc_acq_mode; tdc_det_mode = rhs.tdc_det_mode; detector = rhs.detector;
        return *this;
      }
    };
    typedef std::vector<TDCConditions> TDCConditionsCollection;
    inline TDCConditionsCollection GetTDCConditions(unsigned int run_id) const {
      std::ostringstream os;
      os << "SELECT tdc_id,tdc_address,tdc_acq_mode,tdc_det_mode FROM tdc_conditions"
         << " WHERE run_id=" << run_id;
      std::vector< std::vector<int> > out = Select<int>(os.str());
      if (out.size()==0) {
        std::ostringstream s;
        s << "No TDC conditions found in run " << run_id;
        throw Exception(__PRETTY_FUNCTION__, s.str(), JustWarning);
      }

      TDCConditionsCollection ret;
      for (std::vector< std::vector<int> >::iterator tdc=out.begin(); tdc!=out.end(); tdc++) {
        TDCConditions cond;
        cond.run_id = run_id;
        cond.tdc_id = tdc->at(0);
        cond.tdc_address = tdc->at(1);
        cond.tdc_acq_mode = tdc->at(2);
        cond.tdc_det_mode = tdc->at(3);

        // Retrieve the dector name      
        try {
          std::ostringstream s;
          s << "SELECT detector FROM tdc_conditions"
            << " WHERE tdc_id=" << cond.tdc_id
            << " AND run_id=" << cond.run_id;
          std::vector< std::vector<std::string> > ad = Select<std::string>(s.str());
          if (ad.size()==1) cond.detector = (char*)ad[0][0].c_str();
          else throw Exception(__PRETTY_FUNCTION__, "");
        } catch (Exception& e) {
          std::ostringstream s;
          s << "Failed to retrieve the detector name in run " << run_id
            << " for TDC with base address 0x" << std::hex << cond.tdc_address;
          throw Exception(__PRETTY_FUNCTION__, s.str(), JustWarning);
        }

        ret.push_back(cond);
      }

      return ret;
    }

    inline void SetHVConditions(unsigned short channel_id, unsigned int vmax, unsigned imax) {
      char* err = 0;
      std::ostringstream req;
      unsigned int current_run = GetLastRun();
      req << "INSERT INTO hv (id, run_id, channel, v, i) VALUES (NULL, "
          << current_run << ", "
          << channel_id << ", " << vmax << ", " << imax << ");";
      int rc = sqlite3_exec(fDB, req.str().c_str(), callback, 0, &err);
      if (rc!=SQLITE_OK) {
        std::ostringstream os;
        os << "Error while trying to add a new HV condition" << "\n\t"
           << "SQLite error: " << err;
        throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, 60120);
      }

    }

  private:
    inline void BuildTables() {
      int rc; char* err = 0;
      std::string req;
      // runs table
      req = "CREATE TABLE run(" \
            "id    INTEGER PRIMARY KEY AUTOINCREMENT," \
            "start INTEGER DEFAULT (CAST(strftime('%s', 'now') AS INT))" \
            ");";
      rc = sqlite3_exec(fDB, req.c_str(), callback, 0, &err);
      if (rc!=SQLITE_OK) {
        std::ostringstream os;
        os << "Error while trying to build the run table" << "\n\t"
           << "SQLite error: " << err;
        throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, 60001);
      }
      // bursts table
      req = "CREATE TABLE burst(" \
            "id       INTEGER PRIMARY KEY AUTOINCREMENT," \
            "run_id   INTEGER NOT NULL," \
            "burst_id INTEGER NOT NULL," \
            "start    INTEGER DEFAULT (CAST(strftime('%s', 'now') AS INT))" \
            ");";
      rc = sqlite3_exec(fDB, req.c_str(), callback, 0, &err);
      if (rc!=SQLITE_OK) {
        std::ostringstream os;
        os << "Error while trying to build the burst table" << "\n\t"
           << "SQLite error: " << err;
        throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, 60002);
      }
      // TDC run conditions table
      req = "CREATE TABLE tdc_conditions(" \
            "id           INTEGER PRIMARY KEY AUTOINCREMENT," \
            "run_id       INTEGER NON NULL," \
            "tdc_id       INTEGER NON NULL," \
            "tdc_address  INTEGER NON NULL," \
            "tdc_acq_mode INTEGER NON NULL," \
            "tdc_det_mode INTEGER NON NULL," \
            "detector     CHAR(50)" \
            ");";
      rc = sqlite3_exec(fDB, req.c_str(), callback, 0, &err);
      if (rc!=SQLITE_OK) {
        std::ostringstream os;
        os << "Error while trying to build the TDC/detectors conditions table" << "\n\t"
           << "SQLite error: " << err;
        throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, 60003);
      }
      // HV conditions table
      req = "CREATE TABLE hv(" \
            "id         INTEGER PRIMARY KEY AUTOINCREMENT," \
            "time       INTEGER DEFAULT (CAST(strftime('%s', 'now') AS INT))," \
            "run_id     INTEGER NOT NULL," \
            "channel    INTEGER NOT NULL," \
            "v          INTEGER NOT NULL," \
            "i          INTEGER NOT NULL" \
            ");";
      rc = sqlite3_exec(fDB, req.c_str(), callback, 0, &err);
      if (rc!=SQLITE_OK) {
        std::ostringstream os;
        os << "Error while trying to build the HV conditions table" << "\n\t"
           << "SQLite error: " << err;
        throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, 60004);
      }
    }

    template<class T> inline std::vector< std::vector<T> > Select(std::string req, int num_fields=-1) const {
      if (req.find(';')==std::string::npos) req += ";";
      std::vector< std::vector<T> > out;
      int rc;
      sqlite3_stmt* stmt;
      rc = sqlite3_prepare(fDB, req.c_str(), -1, &stmt, NULL);
      if (rc!=SQLITE_OK) {
        std::ostringstream os;
        os << "Error while preparing the DB to select elements" << "\n\t"
           << "SQLite error: " << sqlite3_errmsg(fDB);
        throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, 60010);
      }
      unsigned int idx = 0;
      while (sqlite3_column_value(stmt, 0)) {
        std::vector<T> line; line.clear();
        if (num_fields<1) num_fields = sqlite3_column_count(stmt);
        for (int i=0; i<num_fields; i++) {
          std::stringstream data; data.str("");
          switch (sqlite3_column_type(stmt, i)) {
            case 1: data << (int)sqlite3_column_int(stmt, i); break;
            case 2: data << (double)sqlite3_column_double(stmt, i); break;
            case 3: data << (char*)sqlite3_column_text(stmt, i); break;
            case 4: data << (char*)sqlite3_column_blob(stmt, i); break;
            //case 5: data << (char*)sqlite3_column_bytes(stmt, i); break;
            default: break;
          }
          T value; data >> value; line.push_back(value);
        }
        if (idx>0) out.push_back(line);
        if (sqlite3_step(stmt)!=SQLITE_ROW) break;
        idx++;
      }
      sqlite3_finalize(stmt);
      return out;
    }

    sqlite3* fDB;
};

#endif
