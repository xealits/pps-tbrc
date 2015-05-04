#include "FPGAHandler.h"

FPGAHandler::FPGAHandler(int port, const char* dev) :
  Client(port), /*USBHandler(dev),*/ USBHandler(FPGA_VENDOR_ID, FPGA_DEVICE_ID),
  fFilename(""), fIsFileOpen(false),
  fIsTDCInReadout(false)
{
  try {
    USBHandler::Init();
  } catch (Exception& e) { e.Dump(); }
  // Read Id code (0b10000100011100001101101011001110 = 0x8470DACE for HPTDCv1.3)
  for (unsigned int i=0; i<NUM_HPTDC; i++) {
    fTDC[i] = new TDC(i, this);
  }
}

FPGAHandler::~FPGAHandler()
{
  CloseFile();
  for (unsigned int i=0; i<NUM_HPTDC; i++) {
    if (fTDC[i]) delete fTDC[i];
  }
}

void
FPGAHandler::OpenFile()
{
  fFilename = GenerateFileName(5);
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
  fOutput.write((char*)&th, sizeof(file_header_t));
  
  for (unsigned int i=0; i<NUM_HPTDC; i++) {
    TDCSetup s = fTDC[i]->GetSetupRegister();
    fOutput.write((char*)&s, sizeof(TDCSetup));
  }
}

int
FPGAHandler::ReadBuffer()
{
  TDCEventCollection ev;
  unsigned int nevts = 0;
  
  if (USBHandler::fIsStopping) {
    std::cout << __PRETTY_FUNCTION__
              << " USB handler in a stopping state! Finishing the readout."
              << std::endl;
    return -1;
  }
  
  for (unsigned int i=0; i<NUM_HPTDC; i++) {
    ev = fTDC[i]->FetchEvents();
    nevts += ev.size();
    for (TDCEventCollection::iterator e=ev.begin(); e!=ev.end(); e++) {
      fOutput.write((char*)&(*e), sizeof(TDCEvent));
    }
  }
  return nevts;
}

void
FPGAHandler::CloseFile()
{
  if (fIsFileOpen) fOutput.close();
}

