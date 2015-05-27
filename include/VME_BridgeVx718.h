/* Interface for CAEN Vx718 VME Bridge */

#ifndef BRIDGEVx718_H 
#define BRIDGEVx718_H

#include "CAENVMElib.h"
#include <iostream>
#include <sstream>
#include <map>

#include "Exception.h"

namespace VME
{
  /// Compatible bridge types
  enum BridgeType { CAEN_V1718, CAEN_V2718 };
  
  /**
   * This class initializes the CAEN V1718 VME bridge in order to control the crate.
   * \brief class defining the VME bridge
   * \author Laurent Forthomme <laurent.forthomme@cern.ch>
   * \author Bob Velghe <bob.velghe@cern.ch>
   * \date Jun 2010
   */
  class BridgeVx718 {
    public:
      /* device : /dev/xxx */
      /**
       * Bridge class constructor
       * \brief Constructor
       * \param[in] device Device identifier on the VME crate
       * \param[in] type Device type (1718/2718)
       */
      BridgeVx718(const char *device, BridgeType type);
      /**
       * Bridge class destructor
       * \brief Destructor
       */
      ~BridgeVx718();

      /**
       * Gives bhandle value
       * \brief Gets bhandle
       * \return bhandle value
       */ 
      inline int32_t GetHandle() const { return fHandle; }
      void CheckConfiguration() const;

      /**
       * \brief Set and control the output lines
       */
      void OutputConf(CVOutputSelect output);
      void OutputOn(CVOutputSelect output);
      void OutputOff(CVOutputSelect output);

      /**
       * \brief Set and read the input lines
       */
      void InputConf(CVInputSelect input);
      void InputRead(CVInputSelect input);

      private:
      /// Map output lines [0,4] to corresponding register.
      std::map<CVOutputSelect,CVOutputRegisterBits> fPortMapping; 
      /// Device handle
      int32_t fHandle;
  };
}

#endif /* BRIDGEV1718_H */
