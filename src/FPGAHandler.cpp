#include "FPGAHandler.h"

FPGAHandler::FPGAHandler(int port, const char* dev) :
  Client(port), fDevice(dev), fFilename(""), fIsFileOpen(false)
{}

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
  if (!(fIsFileOpen=fOutput.is_open()))
    throw Exception(__PRETTY_FUNCTION__, "Error opening the file! Check you have enough permissions to write!", Fatal);
  
  // First we write the header to the file
  file_header_t th;
  th.magic = 0x30535050; // PPS0 in ASCII
  th.run_id = 0;
  th.spill_id = 0;
  th.config = fConfig;
  fOutput.write((char*)&th, sizeof(file_header_t));
}

void
FPGAHandler::SendConfiguration()
{
  // ...
  for (unsigned int i=0; i<fConfig.GetNumWords(); i++) {
    WriteUSB(fConfig.GetWord(i), 32);
  }
}

void
FPGAHandler::ReadConfiguration()
{
  // ...
  int attempts = 0;
  
  do {
    WriteUSB(207, 8); attempts++;
  } while (FetchUSB(8)!=255 and attempts<3);
  
  uint8_t ack; unsigned int i, j;
  uint32_t word = 0x0;
  i = 0; j = 0;
  do {
    ack = (i%2==0) ? 0 : 255;
    uint32_t ret = static_cast<uint32_t>(FetchUSB(8));
    WriteUSB(ack, 8);
    word |= (ret<<i*8);
    if (i%32==0 and i!=0) {
      fConfig.SetWord(j, word);
      word = 0x0;
      j++;
    }
    i++;
  } while (j<fConfig.GetNumWords() and attempts<3);
}

uint32_t
FPGAHandler::FetchUSB(uint8_t size) const
{
  uint32_t out = 0x0;
  // ...
  std::cout << __PRETTY_FUNCTION__ << " fetching from USB:" << std::endl;
  std::cout << " Size: " << static_cast<int>(size) << std::endl;
  return (out&((1<<size)-1));
}

void
FPGAHandler::WriteUSB(uint32_t word, uint8_t size) const
{
  std::cout << __PRETTY_FUNCTION__ << " writing to USB:" << std::endl;
  std::cout << " Size: " << static_cast<int>(size) << std::endl;
  std::cout << " Word: " << word << std::endl;
}
