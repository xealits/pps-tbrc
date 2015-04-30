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
  int data_payload_size = st.st_size-sizeof(file_header_t);
  
  if (!fFile.good()) {
    fFile.close();
    throw Exception(__PRETTY_FUNCTION__, "Can not read file header!", Fatal);
  }
  fFile.read((char*)&fHeader, sizeof(file_header_t));
  if (fHeader.magic != 0x30535050) {
    fFile.close();
    throw Exception(__PRETTY_FUNCTION__, "Wrong magic number!", Fatal);
  }
  std::stringstream s;
  s << "File written on: " << asctime(localtime(&(st.st_mtime)));
  Exception(__PRETTY_FUNCTION__, s.str(), Info).Dump();
  
  for (unsigned int i=0; i<fHeader.num_hptdc; i++) {
    TDCSetup set; fFile.read((char*)&set, sizeof(TDCSetup));
    set.Dump();
    fSetupCollection.push_back(set);
  }
}

FileReader::~FileReader()
{
  if (fFile.is_open()) fFile.close();
}

TDCEvent
FileReader::GetNextEvent()
{
  TDCEvent ev;
  fFile.read((char*)&ev, sizeof(TDCEvent));
  if (fFile) return ev;
  return 0;
}
