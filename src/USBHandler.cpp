#include "USBHandler.h"

USBHandler::USBHandler(const char* dev) :
  fDevice(dev), fHandle(0)
{}

void
USBHandler::Init()
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
  for (int i=0; i<ret; i++) {
    DumpDevice(all_devices[i], o);
  }
  Exception(__PRETTY_FUNCTION__, o.str(), Info).Dump(std::cout);
  //fHandle = libusb_open_device_with_vid_pid(usb_context, 5118, 7424);
  
  libusb_free_device_list(all_devices, 1);
  libusb_exit(usb_context);
}

void
USBHandler::DumpDevice(libusb_device* dev, std::ostream& out)
{
  int ret;
  libusb_device_descriptor desc;
  if ((ret=libusb_get_device_descriptor(dev, &desc))<0) {
    std::ostringstream o; o << "Error while dumping the USB device information. Returned value is " << ret;
    throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
  }
  out << std::endl
      << "====================== USB device dump ======================" << std::endl
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
  out << ")" << std::endl
      << "Vendor/product Id: " << desc.idVendor << " / " << desc.idProduct << std::endl
      << "Number of possible configurations: " << (int)desc.bNumConfigurations << std::endl;
  libusb_config_descriptor *config;
  libusb_get_config_descriptor(dev, 0, &config);
  out << "Interfaces: " << (int)config->bNumInterfaces << std::endl;
  const libusb_interface *inter;
  const libusb_interface_descriptor *interdesc;
  const libusb_endpoint_descriptor *epdesc;
  for (int i=0; i<(int)config->bNumInterfaces; i++) {
    inter = &config->interface[i];
    out << "  Interface " << i << " has " << inter->num_altsetting << " alternate setting(s):" << std::endl;
    for (int j=0; j<inter->num_altsetting; j++) {
      interdesc = &inter->altsetting[j];
      out /*<< "  => Interface Number: " << (int)interdesc->bInterfaceNumber << std::endl*/
          << "     Setting " << j << " with " << (int)interdesc->bNumEndpoints << " endpoint(s):" << std::endl;
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
          case 0x10: out << "device capability"; break;
          default: out << "unrecognized!"; break;
        }
        out << ") at endpoint address 0x" << std::hex << (int)epdesc->bEndpointAddress << std::dec << std::endl;
      }
    }
  }
  libusb_free_config_descriptor(config);
}
