#ifndef RunFile_h
#define RunFile_h

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sqlite3.h>

#include "Exception.h"

static int callback(void* unused, int argc, char* argv[], char* azcolname[])
{
  for (int i=0; i<argc; i++) {
    std::cerr << azcolname[i] << " = " << (argv[i] ? argv[i] : "NULL") << std::endl;
  }
  return 0;
}

/**
 * \brief Handler for the run information file reader
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 3 Aug 2015
 */
class RunFileHandler
{
  public:
    inline RunFileHandler(std::string path="./run_info.dat") {
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

    inline ~RunFileHandler() { sqlite3_close(fDB); }

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
      req << "INSERT INTO burst (id, run_id, burst_id) VALUES (NULL, "
          << current_run << ", "
          << GetLastBurst(current_run)+1 << ")";
      int rc = sqlite3_exec(fDB, req.str().c_str(), callback, 0, &err);
      if (rc!=SQLITE_OK) {
        std::ostringstream os;
        os << "Error while trying to add a new burst" << "\n\t"
           << "SQLite error: " << err;
        throw Exception(__PRETTY_FUNCTION__, os.str(), JustWarning, 60101);
      }
    }
    /// Retrieve the last run acquired
    inline unsigned int GetLastRun() {
      std::vector< std::vector<int> > out = Select<int>("SELECT id FROM run ORDER BY id DESC LIMIT 1");
      if (out.size()!=1) return 0;
      return out[0][0];
    }

    inline int GetLastBurst(unsigned int run) {
      std::ostringstream os;
      os << "SELECT burst_id FROM burst WHERE run_id=" << run << " ORDER BY burst_id DESC LIMIT 1";
      std::vector< std::vector<int> > out = Select<int>(os.str());
      if (out.size()==0) return -1;
      else if (out.size()==1) return out[0][0];
    }

    typedef std::map<unsigned int, unsigned int> BurstInfos;
    /// Retrieve information on a given run (spill IDs / timestamp)
    inline BurstInfos GetRunInfo(unsigned int run) {
      std::ostringstream os;
      BurstInfos ret; ret.clear();
      os << "SELECT burst_id,start FROM burst WHERE run_id=" << run << " ORDER BY burst_id";
      std::vector< std::vector<int> > out = Select<int>(os.str());
      for (std::vector< std::vector<int> >::iterator it=out.begin(); it!=out.end(); it++) {
        ret.insert(std::pair<unsigned int, unsigned int>(it->at(0), it->at(1)));
      }
      return ret;
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
      // Detector conditions table
      req = "CREATE TABLE detectors(" \
            "id     INTEGER PRIMARY KEY AUTOINCREMENT," \
            "run_id INTEGER NOT NULL," \
            "start  INTEGER DEFAULT (CAST(strftime('%s', 'now') AS INT))," \
            "tdc1   CHAR(50)," \
            "tdc2   CHAR(50)," \
            "tdc3   CHAR(50)," \
            "tdc4   CHAR(50)," \
            "tdc5   CHAR(50)" \
            ");";
      rc = sqlite3_exec(fDB, req.c_str(), callback, 0, &err);
      if (rc!=SQLITE_OK) {
        std::ostringstream os;
        os << "Error while trying to build the detectors conditions table" << "\n\t"
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

    template<class T> inline std::vector< std::vector<T> > Select(std::string req, int num_fields=-1) {
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
        for (unsigned int i=0; i<num_fields; i++) {
          std::stringstream data; data.str("");
          switch (sqlite3_column_type(stmt, i)) {
            case 1: data << (int)sqlite3_column_int(stmt, i); break;
            case 2: data << (double)sqlite3_column_double(stmt, i); break;
            case 3: data << (char*)sqlite3_column_text(stmt, i); break;
            case 4: data << (char*)sqlite3_column_blob(stmt, i); break;
            case 5: data << (char*)sqlite3_column_bytes(stmt, i); break;
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
