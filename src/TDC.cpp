#include "TDC.h"

TDC::TDC(unsigned int id, USBHandler* h) : fId(id), fUSB(h)
{
  ReadConfiguration();
}

void
TDC::SendConfiguration()
{
  WriteRegister<TDCSetup>(TDC_SETUP_REGISTER, fSetup);
  WriteRegister<TDCControl>(TDC_CONTROL_REGISTER, fControl);
  WriteRegister<TDCBoundaryScan>(TDC_BS_REGISTER, fBS);
}

void
TDC::ReadConfiguration()
{
  fSetup = ReadRegister<TDCSetup>(TDC_SETUP_REGISTER);
  fControl = ReadRegister<TDCControl>(TDC_CONTROL_REGISTER);
  fBS = ReadRegister<TDCBoundaryScan>(TDC_BS_REGISTER);
}

bool
TDC::CheckFirmwareVersion() const
{
  int attempts = 0;
  unsigned int version = 0;
  do {
    try {
      // First we initiate the communication
      fUSB->Write(FW_START, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
      
      // Request version
      fUSB->Write(FW_GET_VERSION, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      
      // Retrieve the firmware version
      version = fUSB->Fetch(USB_WORD_SIZE);
    
      // Close the communication
      fUSB->Write(FW_STOP, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      
    } catch (Exception& e) { e.Dump(); attempts++; continue; }
  } while (attempts<3);
  return version==TDC_FIRMWARE_VERSION;
}

void
TDC::SoftReset()
{
  int attempts = 0;
  do {
    try {
      fUSB->Write(RESET_WORD, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
    } catch (Exception& e) { e.Dump(); attempts++; continue; }
  } while (attempts<3);
}

TDCEventCollection
TDC::FetchEvents()
{
  int attempts = 0;
  TDCEventCollection ev;
  do {
    try {
      // First we initiate the communication
      fUSB->Write(RO_START, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
      
      // Specify the HPTDC to read
      fUSB->Write(RO_HPTDC_ID, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      fUSB->Write(fId, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
      
      // FIFO depth retrieval
      fUSB->Write(RO_FIFO_SIZE, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      int size = fUSB->Fetch(USB_WORD_SIZE);
      fUSB->Write(0, USB_WORD_SIZE); // FIXME introduce checks on this size
      
      // Specify the number of bytes to retrieve
      unsigned int to_retrieve = std::min(size, 256);
      fUSB->Write(RO_NUM_BYTES, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      fUSB->Write((to_retrieve>>USB_WORD_SIZE)&0xFF, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
      fUSB->Write(to_retrieve&0xFF, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      
      // Retrieve the data
      uint32_t word;
      unsigned int bit = 0;
      for (unsigned int i=0; i<to_retrieve; i++) {
        if (bit>0 and bit==32/USB_WORD_SIZE) {
          TDCEvent e(word);
          word = 0x0;
          ev.push_back(e);
          bit = 0;
        }
        unsigned int ack = (i%2==0) ? 0 : 255;
        unsigned short byte = fUSB->Fetch(USB_WORD_SIZE);
        fUSB->Write(ack, USB_WORD_SIZE);
        word |= ((byte&0xFF)<<(bit*USB_WORD_SIZE));
      }
      
      // Close the communication
      fUSB->Write(CONFIG_STOP, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      
    } catch (Exception& e) { e.Dump(); attempts++; continue; }
  } while (attempts<3);
  
  return ev;
}

template<class T> void
TDC::WriteRegister(unsigned int r, const T& v)
{
  int attempts = 0;
  do {
    try {
      // First we initiate the communication
      fUSB->Write(CONFIG_START, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
      
      // Specify the register to read
      fUSB->Write(CONFIG_REGISTER_NAME, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      fUSB->Write(r, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
      
      // Retrieve the HPTDC identifier
      fUSB->Write(CONFIG_HPTDC_ID, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      fUSB->Write(fId, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
      
      // Specify we want to send a stream
      fUSB->Write(CONFIG_START_STREAM, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      
      // Feed the configuration words
      for (unsigned int i=0; i<fSetup.GetNumWords(); i++) {
        //ack = (i%2==0) ? 0 : 255;
        uint32_t word = v.GetWord(i);
        for (unsigned int j=0; j<WORD_SIZE/USB_WORD_SIZE; j++) {
          fUSB->Write((word>>USB_WORD_SIZE*j)&0xFF, USB_WORD_SIZE);
        }
      }
    
      // Close the communication
      fUSB->Write(CONFIG_STOP, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      
    } catch (Exception& e) { e.Dump(); attempts++; continue; }
  } while (attempts<3);
}

template<class T> T
TDC::ReadRegister(unsigned int r)
{
  T out;
  unsigned int ack, byte, i, j, word;

  int attempts = 0;
  do {
    try {
      // First we initiate the communication
      fUSB->Write(VERIF_START, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
      
      // Specify the register to read
      fUSB->Write(VERIF_REGISTER_NAME, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      fUSB->Write(r, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
      
      // Retrieve the HPTDC identifier
      fUSB->Write(VERIF_HPTDC_ID, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      fUSB->Write(fId, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=0) { attempts++; continue; }
      
      i = j = word = 0;
      // Retrieve the configuration words
      fUSB->Write(VERIF_START_STREAM, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
      do {
        ack = (i%2==0) ? 0 : 255;
        byte = static_cast<uint8_t>(fUSB->Fetch(USB_WORD_SIZE));
        fUSB->Write(ack, USB_WORD_SIZE);
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
      fUSB->Write(VERIF_STOP, USB_WORD_SIZE);
      if (fUSB->Fetch(USB_WORD_SIZE)!=255) { attempts++; continue; }
    } catch (Exception& e) { e.Dump(); attempts++; continue; }
  } while (attempts<3);
  
  return out;
}
