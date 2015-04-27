#ifndef USBHandler_h
#define USBHandler_h

#include <sstream>
#include <iomanip>
#include <libusb-1.0/libusb.h>

#include "Exception.h"

#define USB_WORD_SIZE 8

/**
 * \brief Generic USB communication handler
 * \date 21 Apr 2015
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 */
class USBHandler
{
  public:
    USBHandler(const char* dev);
    inline virtual ~USBHandler() {;}
    
    void Init();
    
    void DumpDevice(libusb_device* dev, int verb=1, std::ostream& out=std::cout);
    
    /// Write a word to the USB device
    void Write(uint32_t word, uint8_t size) const {
      /*std::cout << __PRETTY_FUNCTION__ << " writing to USB:" << std::endl;
      std::cout << " Size: " << static_cast<int>(size) << std::endl;
      std::cout << " Word: 0x" << std::setw(4) << std::hex << word << std::dec << " (";
      for (unsigned int i=0; i<size; i++) std::cout << ((word>>i)&0x1);
      std::cout << ")" << std::endl;*/
    }
    /// Receive a word from the USB device
    uint32_t Fetch(uint8_t size) const {
      uint32_t out = 0x0;
      // ...
      /*std::cout << __PRETTY_FUNCTION__ << " fetching from USB:" << std::endl;
      std::cout << " Size: " << static_cast<int>(size) << std::endl;*/
      return (out&((1<<size)-1));
    }
    
  private:
    std::string fDevice;
    libusb_device_handle *fHandle;
};

#endif
