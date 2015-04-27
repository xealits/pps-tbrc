#include "FPGAHandler.h"

FPGAHandler::FPGAHandler(int port, const char* dev) :
  Client(port), USBHandler(dev), fFilename(""), fIsFileOpen(false),
  fIsTDCInReadout(false)
{
  USBHandler::Init();
  // Read Id code (0b10000100011100001101101011001110 = 0x8470DACE for HPTDCv1.3)
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
  WriteRegister<TDCSetup>(TDC_SETUP_REGISTER, fTDCSetup);
  WriteRegister<TDCControl>(TDC_CONTROL_REGISTER, fTDCControl);
  WriteRegister<TDCBoundaryScan>(TDC_BS_REGISTER, fTDCBS);
}

void
FPGAHandler::ReadConfiguration()
{
  fTDCSetup = ReadRegister<TDCSetup>(TDC_SETUP_REGISTER);
  fTDCControl = ReadRegister<TDCControl>(TDC_CONTROL_REGISTER);
  fTDCBS = ReadRegister<TDCBoundaryScan>(TDC_BS_REGISTER);
}

template<class T> void
FPGAHandler::WriteRegister(unsigned int r, const T& v)
{
  int attempts = 0;
  // First we initiate the communication
  do {
    try {
      // First we initiate the communication
      USBHandler::Write(CONFIG_START, USB_WORD_SIZE); attempts++;
      if (USBHandler::Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
      
      // Specify the register to read
      USBHandler::Write(CONFIG_REGISTER_NAME, USB_WORD_SIZE);
      if (USBHandler::Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      USBHandler::Write(r, USB_WORD_SIZE);
      if (USBHandler::Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
      
      // Retrieve the HPTDC identifier
      USBHandler::Write(CONFIG_HPTDC_ID, USB_WORD_SIZE);
      if (USBHandler::Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      /*USBHandler::Write(r, USB_WORD_SIZE);
      if (USBHandler::Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }*/
      
      // Specify we want to send a stream
      USBHandler::Write(CONFIG_START_STREAM, USB_WORD_SIZE);
      if (USBHandler::Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      
      // Feed the configuration words
      for (unsigned int i=0; i<fTDCSetup.GetNumWords(); i++) {
        //ack = (i%2==0) ? 0 : 255;
        uint32_t word = v.GetWord(i);
        for (unsigned int j=0; j<WORD_SIZE/USB_WORD_SIZE; j++) {
          USBHandler::Write((word>>USB_WORD_SIZE*j)&0xFF, USB_WORD_SIZE);
        }
      }
    
      // Close the communication
      USBHandler::Write(CONFIG_STOP, USB_WORD_SIZE);
      if (USBHandler::Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      
    } catch (Exception& e) { e.Dump(); attempts++; continue; }
  } while (attempts<3);
}

template<class T> T
FPGAHandler::ReadRegister(unsigned int r)
{
  T out;
  unsigned int ack, byte, i, j, word;

  int attempts = 0;
  do {
    try {
      // First we initiate the communication
      USBHandler::Write(VERIF_START, USB_WORD_SIZE); attempts++;
      if (USBHandler::Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
      
      // Specify the register to read
      USBHandler::Write(VERIF_REGISTER_NAME, USB_WORD_SIZE);
      if (USBHandler::Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      USBHandler::Write(r, USB_WORD_SIZE);
      if (USBHandler::Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
      
      // Retrieve the HPTDC identifier
      USBHandler::Write(VERIF_HPTDC_ID, USB_WORD_SIZE);
      if (USBHandler::Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      /*USBHandler::Write(r, USB_WORD_SIZE);
      if (USBHandler::Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }*/
      
      i = j = word = 0;
      // Retrieve the configuration words
      USBHandler::Write(VERIF_START_STREAM, USB_WORD_SIZE);
      if (USBHandler::Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      do {
        ack = (i%2==0) ? 0 : 255;
        byte = static_cast<uint8_t>(USBHandler::Fetch(USB_WORD_SIZE));
        USBHandler::Write(ack, USB_WORD_SIZE);
        word |= (byte<<i*USB_WORD_SIZE);
        if (i%WORD_SIZE==0 and i!=0) {
          out.SetWord(j, word);
          word = 0x0; j++;
        }
        i++;
      }
      while (j<out.GetNumWords());
      i = 0;
      
      // Close the communication
      USBHandler::Write(VERIF_STOP, USB_WORD_SIZE);
      if (USBHandler::Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
    } catch (Exception& e) { e.Dump(); attempts++; continue; }
  } while (attempts<3);
  
  return out;
}

