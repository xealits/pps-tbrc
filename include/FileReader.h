#ifndef FileReader_h
#define FileReader_h

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>

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
    /**
     * \brief Class constructor
     * \param[in] name Path to the file to read
     * \param[in] ro Data readout mode (continuous storage or trigger matching)
     */
    FileReader(std::string name, const VME::ReadoutMode& ro);
    ~FileReader();
    
    inline unsigned int GetNumTDCs() const { return fHeader.num_hptdc; }
    
    bool GetNextEvent(VME::TDCEvent*);
    /**
     * \brief Fetch the next full measurement on a given channel
     * \param[in] channel_id Unique identifier of the channel number to retrieve
     * \param[out] m A full measurement with leading, trailing times, ...
     * \return A boolean stating the success of retrieval operation
     */
    bool GetNextMeasurement(unsigned int channel_id, VME::TDCMeasurement* m);
    
  private:
    std::ifstream fFile;
    file_header_t fHeader;
    VME::ReadoutMode fReadoutMode;
};

#endif
