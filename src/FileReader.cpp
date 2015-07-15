#include "FileReader.h"

FileReader::FileReader(std::string file)
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
  
  if (!fFile.good()) {
    fFile.close();
    throw Exception(__PRETTY_FUNCTION__, "Can not read file header!", Fatal);
  }
  fFile.read((char*)&fHeader, sizeof(file_header_t));
  fNumEvents = (st.st_size-sizeof(file_header_t))/sizeof(uint32_t);
  if (fHeader.magic!=0x30535050) {
    fFile.close();
    throw Exception(__PRETTY_FUNCTION__, "Wrong magic number!", Fatal);
  }
  fReadoutMode = fHeader.acq_mode;

  std::stringstream s;
  char buff[80];
  strftime(buff, 80, "%c", localtime(&(st.st_mtime)));
  s << "File written on: " << buff << "\n\t"
    << "Run number: " << fHeader.run_id << "\n\t"
    << "Readout mode: " << fHeader.acq_mode << "\n\t"
    << "Number of events: " << (st.st_size-sizeof(file_header_t))/sizeof(uint32_t);
  PrintInfo(s.str());
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
#ifdef DEBUG
  std::cerr << "Event type: " << ev->GetType();
  if (ev->GetType()==VME::TDCEvent::TDCMeasurement)
    std::cerr << "  channel " << std::setw(2) << ev->GetChannelId() << "  trail? " << ev->IsTrailing();
  std::cerr << std::endl;
#endif
  if (fFile.eof()) return false;
  return true;
}

bool
FileReader::GetNextMeasurement(unsigned int channel_id, VME::TDCMeasurement* mc)
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
    bool has_lead = false, has_trail = false, has_error = false;
    while (true) {
      if (!GetNextEvent(&ev)) return false;
      if (ev.GetChannelId()!=channel_id) { continue; }

      ec.push_back(ev);

      switch (ev.GetType()) {
        case VME::TDCEvent::TDCHeader:
        case VME::TDCEvent::TDCTrailer:
          //std::cerr << "Event Id=" << ev.GetEventId() << std::endl;
          break;
        case VME::TDCEvent::TDCMeasurement:
          //std::cerr << "Leading measurement? " << (!ev.IsTrailing()) << std::endl;
          if (ev.IsTrailing()) has_trail = true;
          else has_lead = true;
          break;
        case VME::TDCEvent::GlobalHeader:
        case VME::TDCEvent::GlobalTrailer:
        case VME::TDCEvent::ETTT:
          break;
        case VME::TDCEvent::TDCError:
          has_error = true;
          std::cerr << " ---> Error flags: " << ev.GetErrorFlags() << std::endl;
          break;
        case VME::TDCEvent::Filler:
          break;
      }
      if (has_lead and has_trail) break;
    }
    if (has_error) throw Exception(__PRETTY_FUNCTION__, "Measurement has at least one error word.", JustWarning);
  }
  else if (fReadoutMode==VME::TRIG_MATCH) {
    bool has_error = false;
    while (true) {
      if (!GetNextEvent(&ev)) return false;
      if (ev.GetType()==VME::TDCEvent::TDCMeasurement) {
        if (ev.GetChannelId()!=channel_id) { continue; }
      }
      ec.push_back(ev);

      switch (ev.GetType()) {
        case VME::TDCEvent::TDCError: has_error = true; break;
        default: break;
      }

      //ev.Dump();
      /*std::cout << "0x" << std::hex << ev.GetType();
      if (ev.GetType()==VME::TDCEvent::TDCMeasurement) {
        std::cout << "\t" << ev.IsTrailing() << "\t" << ev.GetChannelId();
      }
      std::cout << std::endl;*/

      if (ev.GetType()==VME::TDCEvent::GlobalTrailer) break;
    }
    if (has_error) throw Exception(__PRETTY_FUNCTION__, "Measurement has at least one error word.", JustWarning);
  }
  else {
    std::ostringstream os;
    os << "Unrecognized readout/acquisition mode: " << fReadoutMode;
    throw Exception(__PRETTY_FUNCTION__, os.str(), Fatal);
  }
  mc->SetEventsCollection(ec);
  return true;
}

