/* Interface for CAEN V1718 VME - USB 2.0 Bridge */

#ifndef BRIDGEV1718_H 
#define BRIDGEV1718_H

#include "CAENVMElib.h"
#include <iostream>
#include <map>

/*! \class Bridge
 * \brief class defining the VME bridge
 *  This class initializes the CAEN V1718 VME bridge in order to control the crate.
 */
class VMEBridgeV1718 {
  public:
    /* device : /dev/xxx */
    /**
     * Bridge class constructor
     * \brief Constructor
     * \param[in] Device identifier on the VME crate
     */
    VMEBridgeV1718(const char *device);
    /**
     * Bridge class destructor
     * \brief Destructor
     */
    ~VMEBridgeV1718();
    /**
     * Gives bhandle value
     * \brief Gets bhandle
     * \return bhandle value
     */ 
    int32_t getBHandle();

    /**
     * \brief Set and control the output lines
     * \return 0 upon success, -1 otherwise
     */
    int outputConf(CVOutputSelect output);
    int outputOn(CVOutputSelect output);
    int outputOff(CVOutputSelect output);

    /**
     * \brief Set and read the input lines
     * \return 0 upon success, -1 otherwise
     */
    int inputConf(CVInputSelect input);
    int inputRead(CVInputSelect input);

    private:
    /// Map output lines [0,4] to corresponding register.
    std::map<CVOutputSelect,CVOutputRegisterBits> map_port; 
    /// Device handle
    int32_t bhandle;
};

#endif /* BRIDGEV1718_H */
