#ifndef FileReader_h
#define FileReader_h

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <iomanip>

#include "FileConstants.h"
#include "Exception.h"

#include "VME_TDCMeasurement.h"

/**
 * \brief Handler for a TDC output file readout
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date Jun 2015
 */
class FileReader
{
  public:
    inline FileReader() {;}
    /**
     * \brief Class constructor
     * \param[in] name Path to the file to read
     * \param[in] ro Data readout mode (continuous storage or trigger matching)
     */
    FileReader(std::string name);
    ~FileReader();
    
    void Open(std::string name);
    inline bool IsOpen() const { return fFile.is_open(); }
    inline void Clear() { fFile.clear(); fFile.seekg(sizeof(file_header_t), std::ios::beg); }

    void Dump() const;    
    inline unsigned int GetNumTDCs() const { return fHeader.num_hptdc; }
    inline unsigned int GetRunId() const { return fHeader.run_id; }
    inline unsigned int GetBurstId() const { return fHeader.spill_id; }
    inline unsigned int GetAcquisitionMode() const { return fHeader.acq_mode; }
    inline unsigned int GetDetectionMode() const { return fHeader.det_mode; }
    
    unsigned long GetNumEvents() const { return fNumEvents; }
    bool GetNextEvent(VME::TDCEvent*);
    /**
     * \brief Fetch the next full measurement on a given channel
     * \param[in] channel_id Unique identifier of the channel number to retrieve
     * \param[out] m A full measurement with leading, trailing times, ...
     * \return A boolean stating the success of retrieval operation
     */
    bool GetNextMeasurement(unsigned int channel_id, VME::TDCMeasurement* mc);
    
  private:
    std::ifstream fFile;
    file_header_t fHeader;
    VME::AcquisitionMode fReadoutMode;
    time_t fWriteTime;
    unsigned long fNumEvents;
};

#endif
