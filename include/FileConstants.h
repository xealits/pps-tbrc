#ifndef FileConstants_h
#define FileConstants_h

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <fstream>

#include "VME_TDCEvent.h"

/**
 * General header to store in each collected data file for offline readout. It
 * enable any reader to retrieve the run/spill number, as well as the HPTDC
 * configuration during data collection.
 * \brief Header to the output files
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 14 Apr 2015 
 */
struct file_header_t {
  uint32_t magic;
  uint32_t run_id;
  uint32_t spill_id;
  uint8_t num_hptdc;
  VME::AcquisitionMode acq_mode;
  VME::DetectionMode det_mode;
};

/// Generate a random string of fixed length for file name
inline std::string GenerateString(const size_t len=5)
{
  std::string out;
  srand(time(NULL));
  const char az[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
  out = "";
  for (size_t i=0; i<len; i++) { out += az[rand()%(sizeof(az)-1)]; }
  return out;
}

/**
 * \brief Redirect outputs to another output stream
 */
class Logger
{
  public:
    inline Logger(std::ostream& lhs, std::ostream& rhs=std::cout) : fStream(rhs), fBuffer(fStream.rdbuf()) {
      fStream.rdbuf(lhs.rdbuf());
    }
    inline ~Logger() { fStream.rdbuf(fBuffer); }

  private:
    std::ostream& fStream;
    std::streambuf* const fBuffer;
};

/**
 * \brief Redirect output stream to a string
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 3 Aug 2015
 */
class LogRedirector
{
  public:
    inline LogRedirector(std::ostream& stm=std::cout) : fSS(), fRedirect(fSS, stm) {;}
    inline std::string contents() const { return fSS.str(); }

  private:
    std::ostringstream fSS;
    const Logger fRedirect;
};

#endif
