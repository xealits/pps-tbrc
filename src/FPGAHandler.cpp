#include "FPGAHandler.h"

FPGAHandler::FPGAHandler(int port, const char* dev) :
  Client(port), USBHandler(dev), fFilename(""), fIsFileOpen(false),
  fIsTDCInReadout(false)
{
  USBHandler::Init();
  // Read Id code (0b10000100011100001101101011001110 = 0x8470DACE for HPTDCv1.3)
  for (unsigned int i=0; i<NUM_HPTDC; i++) {
    fTDC[i] = new TDC(i, this);
  }
}

FPGAHandler::~FPGAHandler()
{
  CloseFile();
  for (unsigned int i=0; i<NUM_HPTDC; i++) {
    delete fTDC[i];
  }
}

void
FPGAHandler::OpenFile()
{
  // Generate a random file name
  srand(time(NULL));
  const char az[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
  std::string filename; const size_t len = 5;
  fFilename = "events_";
  //for (size_t i=0; i<len; i++) { fFilename += az[rand()%(sizeof(az)-1)]; } //FIXME commented out for debugging purposes...
  fFilename += ".dat";
  
  std::cout << "Filename: " << fFilename << std::endl;
  
  fOutput.open(fFilename.c_str(), std::fstream::out|std::ios::binary);
  if (!(fIsFileOpen=fOutput.is_open()))
    throw Exception(__PRETTY_FUNCTION__, "Error opening the file! Check you have enough permissions to write!", Fatal);
  
  // First we write the header to the file
  file_header_t th;
  th.magic = 0x30535050; // PPS0 in ASCII
  th.run_id = 0;
  th.spill_id = 0;
  th.num_hptdc = NUM_HPTDC;
  for (unsigned int i=0; i<NUM_HPTDC; i++) {
    th.config[i] = fTDC[i]->GetSetupRegister();
  }
  fOutput.write((char*)&th, sizeof(file_header_t));

}

void
FPGAHandler::CloseFile()
{
  if (fIsFileOpen) fOutput.close();
}

