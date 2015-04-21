#ifndef USBHandler_h
#define USBHandler_h

#include <usb.h>

#define USB_WORD_SIZE 8

class USBHandler
{
  public:
    USBHandler(const char* dev) : fDevice(dev) {
    }
    virtual ~USBHandler() {;}
    
  protected:
    /// Write a word to the USB device
    void WriteUSB(uint32_t word, uint8_t size) const {
      std::cout << __PRETTY_FUNCTION__ << " writing to USB:" << std::endl;
      std::cout << " Size: " << static_cast<int>(size) << std::endl;
      std::cout << " Word: " << word << std::endl;
    }
    /// Receive a word from the USB device
    uint32_t FetchUSB(uint8_t size) const {
      uint32_t out = 0x0;
      // ...
      std::cout << __PRETTY_FUNCTION__ << " fetching from USB:" << std::endl;
      std::cout << " Size: " << static_cast<int>(size) << std::endl;
      return (out&((1<<size)-1));
    }
    
  private:
    std::string fDevice;
};

#endif
