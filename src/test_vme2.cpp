#include "VMEReader.h"
#include "FileConstants.h"

#include "Client.h"
#include "Socket.h"
#include "OnlineDBHandler.h"

#include "VME_BridgeVx718.h"
#include "VME_FPGAUnitV1495.h"
#include "VME_IOModuleV262.h"
#include "VME_CFDV812.h"
#include "VME_CAENETControllerV288.h"
#include "VME_TDCV1x90.h"

#include "NIM_HVModuleN470.h"

#include <map>
#include "tinyxml2.h"

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <ctime>
#include <signal.h>


using namespace std;

VMEReader* vme;
int gEnd = 0;



int main(int argc, char *argv[]) {
  
	uint32_t address =  0x00ee0000;
	bool with_socket = true;

	vme = new VMEReader("/dev/a2818_0", VME::CAEN_V2718, with_socket);
    	void AddFPGAUnit(address);
	VME::FPGAUnitV1495* fpga = vme->GetFPGAUnit();
	const bool use_fpga = (fpga!=0);

	


	uint32_t Vth = 10 ;
	if (use_fpga) {
		fpga->SetThresholdVoltage(Vth);
   	   	std::cout << "The readback value of the Threshold Voltage is: " << fpga->GetThresholdVoltage() << std::endl;
	}
  
  	return 0;
}
