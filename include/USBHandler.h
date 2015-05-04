#ifndef USBHandler_h
#define USBHandler_h

#include <sstream>
#include <iomanip>
#include <libusb-1.0/libusb.h>

#include "Exception.h"

#define USB_WORD_SIZE 8

//#define DEBUG

/**
 * \brief Generic USB communication handler
 * \date 21 Apr 2015
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \ingroup FPGA
 */
class USBHandler
{
  public:
    USBHandler(const char* dev);
    USBHandler(const unsigned int vendor_id, const unsigned int product_id);
    virtual ~USBHandler();
    
    void Init();
    
    void DumpDevice(libusb_device* dev, int verb=1, std::ostream& out=std::cout);
    
    /// Write a word to the USB device
    void Write(uint32_t word, uint8_t size) const;
    /// Receive a word from the USB device
    uint32_t Fetch(uint8_t size) const;

  protected:
    bool fIsStopping;
    
  private:
    std::string fDevice;
    /// Device's vendor identifier
    unsigned int fVendorId;
    /// Device's product number
    unsigned int fProductId;
    libusb_device_handle *fHandle;
};

#endif
