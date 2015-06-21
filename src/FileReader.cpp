#include "FileReader.h"

FileReader::FileReader(std::string file, const VME::ReadoutMode& ro) :
  fReadoutMode(ro)
{
  fFile.open(file.c_str(), std::ios::in|std::ios::binary);
  
  if (!fFile.is_open()) {
    std::stringstream s;
    s << "Error while trying to open the file \""
      << file << "\" for reading!";
    throw Exception(__PRETTY_FUNCTION__, s.str(), Fatal);
  }
  
  struct stat st;
  // Retrieve the file size
  if (stat(file.c_str(), &st) == -1) {
    std::stringstream s;
    s << "Error retrieving size of \"" << file << "\"!";
    fFile.close();
    throw Exception(__PRETTY_FUNCTION__, s.str(), Fatal);
  }
  //int data_payload_size = st.st_size-sizeof(file_header_t);
  
  if (!fFile.good()) {
    fFile.close();
    throw Exception(__PRETTY_FUNCTION__, "Can not read file header!", Fatal);
  }
  fFile.read((char*)&fHeader, sizeof(file_header_t));
  if (fHeader.magic!=0x30535050) {
    fFile.close();
    throw Exception(__PRETTY_FUNCTION__, "Wrong magic number!", Fatal);
  }
  std::stringstream s;
  char buff[80];
  strftime(buff, 80, "%c", localtime(&(st.st_mtime)));
  s << "File written on: " << buff << "\n\t"
    << "Run number: " << fHeader.run_id << "\n\t" 
    << "Number of events: " << (st.st_size-sizeof(file_header_t))/sizeof(uint32_t);
  PrintInfo(s.str());
  exit(0);
}

FileReader::~FileReader()
{
  if (fFile.is_open()) fFile.close();
}

bool
FileReader::GetNextEvent(VME::TDCEvent* ev)
{
  uint32_t buffer;
  fFile.read((char*)&buffer, sizeof(uint32_t));
  ev->SetWord(buffer);
  if (fFile.eof()) return false;
  return true;
}

bool
FileReader::GetNextMeasurement(unsigned int channel_id, VME::TDCMeasurement* m)
{
  VME::TDCEvent ev;
  std::vector<VME::TDCEvent> ec;
  /*do { GetNextEvent(&ev);
    std::cout << ev.GetChannelId() << " type: " << ev.GetType() << std::endl; 
  } while (ev.GetType()!=VME::TDCEvent::TDCHeader);*/
  /*do {
    GetNextEvent(&ev);
    if (ev.GetChannelId()!=channel_id) continue;
    ec.push_back(ev);
    std::cout << ev.GetType() << std::endl;
  } while (ev.GetType()!=VME::TDCEvent::TDCHeader);*/

  if (fReadoutMode==VME::CONT_STORAGE) {
    bool has_lead = false, has_trail = false;
    while (GetNextEvent(&ev)) {
      if (ev.GetChannelId()!=channel_id) continue;
      if (ev.GetType()==VME::TDCEvent::TDCHeader) continue;

      ec.push_back(ev);

      if (ev.IsTrailing()) has_trail = true;
      else has_lead = true;
      if (has_lead and has_trail) break;
    }
  }
  //std::cout << "--> " << ec.size() << std::endl;
  m->SetEventsCollection(ec);
  return true;
}

