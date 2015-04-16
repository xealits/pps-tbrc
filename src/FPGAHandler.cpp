#include "FPGAHandler.h"

FPGAConfiguration::FPGAConfiguration() :
  fWord(0)
{}

////////////////////////////////////////////////////////////////////////////////

FPGAHandler::FPGAHandler(int port, const char* dev) :
  Listener(port), fDevice(dev), fFilename(""), fIsFileOpen(false)
{  
  try { OpenFile(); } catch (Exception& e) { e.Dump(); }
}

FPGAHandler::~FPGAHandler()
{
  if (fIsFileOpen) {
    fOutput.close();
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
  if (!fOutput.is_open())
    throw Exception(__PRETTY_FUNCTION__, "Error opening the file! Check you have enough permissions to write!", Fatal);
  
  // First we write the header to the file
  file_header_t th;
  th.magic = 0x50505330; // PPS0 in ASCII
  th.run_id = 0;
  th.spill_id = 0;
  fOutput.write((char*)&th, sizeof(file_header_t));
}

void
FPGAHandler::SendConfiguration(const FPGAConfiguration& c)
{
}

FPGAConfiguration
FPGAHandler::ReadConfiguration()
{
  FPGAConfiguration c;
  return c;
}
