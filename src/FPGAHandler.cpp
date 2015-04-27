#include "FPGAHandler.h"

FPGAHandler::FPGAHandler(int port, const char* dev) :
  Client(port), USBHandler(dev), fFilename(""), fIsFileOpen(false),
  fIsTDCInReadout(false)
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
  th.config = fTDCSetup;
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
  
  do {
    // Initiate the communication
    USBHandler::Write(CONFIG_START, USB_WORD_SIZE);
    if (USBHandler::Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
    
    // Specify we want to send a stream
    USBHandler::Write(CONFIG_START_STREAM, USB_WORD_SIZE);
    if (USBHandler::Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
    
    // Feed the configuration words
    for (unsigned int i=0; i<fTDCSetup.GetNumWords(); i++) {
      //ack = (i%2==0) ? 0 : 255;
      uint32_t word = fTDCSetup.GetWord(i);
      for (unsigned int j=0; j<WORD_SIZE/USB_WORD_SIZE; j++) {
        USBHandler::Write((word>>USB_WORD_SIZE*j)&0xFF, USB_WORD_SIZE);
      }
    }
  
    // Close the communication
    USBHandler::Write(CONFIG_STOP, USB_WORD_SIZE);
    if (USBHandler::Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
  } while (attempts<3);
}

void
FPGAHandler::ReadConfiguration()
{
  TDCSetup c;
  // ...
  unsigned int ack, byte, i=0, j=0, word=0x0;
  int attempts = 0;
  
  // First we initiate the communication
  do {
    USBHandler::Write(VERIF_START_STREAM, USB_WORD_SIZE); attempts++;
    if (USBHandler::Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
  
    // Retrieve the configuration words
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
    }
    while (j<fTDCSetup.GetNumWords());
    
    // Close the communication
    USBHandler::Write(VERIF_STOP, USB_WORD_SIZE);
    if (USBHandler::Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
  } while (attempts<3);
  
  fTDCSetup = c;
}

void
FPGAHandler::SetRegister(const TDCControl::RegisterName& r, unsigned int v)
{
  switch (r) {
    case TDCControl::R_GlobalReset: fTDCControl.SetGlobalReset(v&0x1); break;
    case TDCControl::R_DLLReset: fTDCControl.SetDLLReset(v&0x1); break;
    case TDCControl::R_PLLReset: fTDCControl.SetPLLReset(v&0x1); break;
    default:
      break;
  }
}


unsigned int
FPGAHandler::ReadRegister(const TDCControl::RegisterName& r)
{
  switch (r) {
    case TDCControl::R_GlobalReset: return fTDCControl.GetGlobalReset();
    case TDCControl::R_DLLReset: return fTDCControl.GetDLLReset();
    case TDCControl::R_PLLReset: return fTDCControl.GetPLLReset();
    default:
      return -1;
  }
}

void
FPGAHandler::ReadBoundaryScanRegister()
{
  TDCBoundaryScanRegister bsr;
  
  fTDCBSR = bsr;
}
