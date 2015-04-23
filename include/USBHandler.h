#ifndef USBHandler_h
#define USBHandler_h

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
    USBHandler(const char* dev) :
      fDevice(dev), fHandle(0)
    {}
    virtual ~USBHandler() {;}
    
    void Init()
    {
      int ret;
      /// Pointer to a pointer of devices used to retrieve a list of them
      libusb_device** all_devices;
      /// A libusb session
      libusb_context* usb_context = NULL;
      if ((ret=libusb_init(&usb_context))<0) {
        std::ostringstream o; o << "Error while initializing the USB protocol. Returned value is " << ret;
        throw Exception(__PRETTY_FUNCTION__, o.str(), Fatal);
      }
      libusb_set_debug(usb_context, 3);
      if ((ret=libusb_get_device_list(usb_context, &all_devices))<0) {
        std::ostringstream o; o << "Error while extracting the list of USB devices. Returned value is " << ret;
        throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
      }
      std::ostringstream o; o << "Dumping the list of USB devices attached to this machine:\n\t";
      for (unsigned int i=0; i<ret; i++) {
        DumpDevice(all_devices[i], o);
      }
      Exception(__PRETTY_FUNCTION__, o.str(), Info).Dump();
      //fHandle = libusb_open_device_with_vid_pid(usb_context, 5118, 7424);
      
      libusb_free_device_list(all_devices, 1);
      libusb_exit(usb_context);
    }
    
    void DumpDevice(libusb_device *dev, std::ostream& out=std::cout)
    {
      int ret;
      libusb_device_descriptor desc;
      if ((ret=libusb_get_device_descriptor(dev, &desc))<0) {
        std::ostringstream o; o << "Error while dumping the USB device information. Returned value is " << ret;
        throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
      }
      out << endl
          << "====================== USB device dump ======================" << endl
          << "Device Class: " << (int)desc.bDeviceClass << " (";
      switch ((int)desc.bDeviceClass) {
        case 0x0: out << "[class determined from device's interface descriptor]"; break;
        case 0x1: out << "interface/audio"; break;
        case 0x2: out << "device-interface/communications"; break;
        case 0x3: out << "interface/human interface device"; break;
        case 0x5: out << "interface/physical"; break;
        case 0x6: out << "interface/image"; break;
        case 0x7: out << "interface/printer"; break;
        case 0x8: out << "interface/mass storage"; break;
        case 0x9: out << "device/HUB"; break;
        case 0xA: out << "interface/CDC data"; break;
        case 0xB: out << "interface/smart card"; break;
        case 0xD: out << "interface/content security"; break;
        case 0xE: out << "interface/video"; break;
        case 0xF: out << "interface/personal healthcare"; break;
        case 0x10: out << "interface/audio-video device"; break;
        case 0x11: out << "device/billboard device"; break;
        case 0xDC: out << "device-interface/diagnostic device"; break;
        case 0xE0: out << "interface/wireless controller"; break;
        case 0xEF: out << "device-interface/miscellaneous"; break;
        case 0xFE: out << "interface/application specific"; break;
        case 0xFF: out << "device-interface/vendor specific"; break;
        default: out << "unrecognized!"; break;
      }
      out << ")" << endl
          << "Vendor/product Id: " << desc.idVendor << " / " << desc.idProduct << endl
          << "Number of possible configurations: " << (int)desc.bNumConfigurations << endl;
      libusb_config_descriptor *config;
      libusb_get_config_descriptor(dev, 0, &config);
      out << "Interfaces: " << (int)config->bNumInterfaces << endl;
      const libusb_interface *inter;
      const libusb_interface_descriptor *interdesc;
      const libusb_endpoint_descriptor *epdesc;
      for (int i=0; i<(int)config->bNumInterfaces; i++) {
        inter = &config->interface[i];
        out << "  Interface " << i << " has " << inter->num_altsetting << " alternate setting(s):" << endl;
        for (int j=0; j<inter->num_altsetting; j++) {
          interdesc = &inter->altsetting[j];
          out /*<< "  => Interface Number: " << (int)interdesc->bInterfaceNumber << endl*/
              << "     Setting " << j << " with " << (int)interdesc->bNumEndpoints << " endpoint(s):" << endl;
          for (int k=0; k<(int)interdesc->bNumEndpoints; k++) {
            epdesc = &interdesc->endpoint[k];
            out << "     - Descriptor Type: " << (int)epdesc->bDescriptorType << " (";
            switch ((int)epdesc->bDescriptorType) {
              case 0x1: out << "device"; break;
              case 0x2: out << "configuration"; break;
              case 0x3: out << "string"; break;
              case 0x4: out << "interface"; break;
              case 0x5: out << "endpoint"; break;
              case 0x6: out << "device qualifier"; break;
              case 0x7: out << "other speed configuration"; break;
              case 0x8: out << "interface power (obsolete)"; break;
              case 0x9: out << "on-the-go"; break;
            }
            out << ") at endpoint address 0x" << hex << (int)epdesc->bEndpointAddress << endl;
          }
        }
      }
	    libusb_free_config_descriptor(config);
    }
    
  protected:
    /// Write a word to the USB device
    void WriteUSB(uint32_t word, uint8_t size) const {
      /*std::cout << __PRETTY_FUNCTION__ << " writing to USB:" << std::endl;
      std::cout << " Size: " << static_cast<int>(size) << std::endl;
      std::cout << " Word: 0x" << std::setw(4) << std::hex << word << std::dec << " (";
      for (unsigned int i=0; i<size; i++) std::cout << ((word>>i)&0x1);
      std::cout << ")" << std::endl;*/
    }
    /// Receive a word from the USB device
    uint32_t FetchUSB(uint8_t size) const {
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
