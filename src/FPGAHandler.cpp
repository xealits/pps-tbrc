#include "FPGAHandler.h"

FPGAHandler::FPGAHandler(int port, const char* dev) :
  Client(port), USBHandler(dev), fFilename(""), fIsFileOpen(false)
{
  USBHandler::Init();
}

FPGAHandler::~FPGAHandler()
{
  CloseFile();
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
FPGAHandler::CloseFile()
{
  if (fIsFileOpen) fOutput.close();
}

void
FPGAHandler::SendConfiguration()
{
  // ...
  int attempts = 0;
  
  // First we initiate the communication
  do {
    USBHandler::Write(199, USB_WORD_SIZE); attempts++;
  } while (USBHandler::Fetch(USB_WORD_SIZE)!=255 and attempts<3);
  
  // Then we feed the configuration words
  for (unsigned int i=0; i<fConfig.GetNumWords(); i++) {
    //ack = (i%2==0) ? 0 : 255;
    uint32_t word = fConfig.GetWord(i);
    for (unsigned int j=0; j<WORD_SIZE/USB_WORD_SIZE; j++) {
      USBHandler::Write((word>>USB_WORD_SIZE*j)&0xFF, USB_WORD_SIZE);
    }
  }
  
  // Finally we close the communication
  do {
    USBHandler::Write(60, USB_WORD_SIZE); attempts++;
  } while (USBHandler::Fetch(USB_WORD_SIZE)!=255 and attempts<3);
}

void
FPGAHandler::ReadConfiguration()
{
  TDCConfiguration c;
  // ...
  unsigned int ack, byte, i=0, j=0, word=0x0;
  int attempts = 0;
  
  // First we initiate the communication
  do {
    USBHandler::Write(207, USB_WORD_SIZE); attempts++;
  } while (USBHandler::Fetch(USB_WORD_SIZE)!=255 and attempts<3);
  
  // Then we retrieve the configuration words
  do {
    ack = (i%2==0) ? 0 : 255;
    byte = static_cast<uint8_t>(USBHandler::Fetch(USB_WORD_SIZE));
    USBHandler::Write(ack, USB_WORD_SIZE);
    word |= (byte<<i*USB_WORD_SIZE);
    if (i%WORD_SIZE==0 and i!=0) {
      c.SetWord(j, word);
      word = 0x0; j++;
    }
    i++;
  } while (j<fConfig.GetNumWords() and attempts<3);
  
  // Finally we close the communication
  do {
    USBHandler::Write(15, USB_WORD_SIZE); attempts++;
  } while (USBHandler::Fetch(USB_WORD_SIZE)!=255 and attempts<3);
  
  fConfig = c;
}

