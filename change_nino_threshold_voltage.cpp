#include "VMEReader.h"
#include "VME_FPGAUnitV1495.h"

using namespace std;

VMEReader* vme;
int gEnd = 0;

int main(int argc, char *argv[]) {
  uint32_t address =  0x00ee0000;
  bool with_socket = true;

  vme = new VMEReader("/dev/a2818_0", VME::CAEN_V2718, with_socket);
  try { vme->AddFPGAUnit(address); } catch (Exception& e) { e.Dump(); }
  VME::FPGAUnitV1495* fpga = vme->GetFPGAUnit();
  const bool use_fpga = (fpga!=0);

  uint32_t Vth = 10 ;
  if (use_fpga) {
    fpga->SetThresholdVoltage(Vth);
    std::cout << "The readback value of the Threshold Voltage is: " << fpga->GetThresholdVoltage() << std::endl;
  }
  
  return 0;
}
