#include "VME_TDCV1x90.h"

namespace VME
{
  TDCV1x90::TDCV1x90(int32_t bhandle, uint32_t baseaddr) :
    GenericBoard<TDCV1x90Register,cvA32_U_DATA>(bhandle, baseaddr),
    fVerb(2), fAcquisitionMode(TRIG_MATCH), fDetectionMode(TRAILEAD) 
  {
    //event_nb = 0;
    //event_max = 1024;

    fBuffer = (uint32_t*)malloc(32*1024*1024); // 32MB of buffer!
    if (fBuffer==NULL) {
      throw Exception(__PRETTY_FUNCTION__, "Output buffer has not been allocated!", Fatal);
    }
  
    try {
      CheckConfiguration();
    } catch (Exception& e) {
      e.Dump();
      throw Exception(__PRETTY_FUNCTION__, "Wrong configuration!", Fatal);
    }
    
    SoftwareReset();
    //SoftwareClear();
   
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::EN_ALL_CHANNEL);
    } catch (Exception& e) { e.Dump(); }
 
    SetAcquisitionMode(fAcquisitionMode);
    SetDetectionMode(fDetectionMode);
    SetLSBTraileadEdge(r25ps);
    SetRCAdjust(0,0);
    SetRCAdjust(1,0);

    GlobalOffset offs; offs.fine = 0x0; offs.coarse = 0x0;
    SetGlobalOffset(offs); // coarse and fine set
    //GetGlobalOffset();

    //SetBLTEventNumberRegister(1); // FIXME find good value!
    SetTDCEncapsulation(false);
    SetErrorMarks(true);
    SetETTT(true);
    SetWindowWidth(2045); // in units of clock cycles
    SetWindowOffset(-2050); // in units of clock cycles
    //SetPairModeResolution(0,0x4);
    SetPoI(0xFFFF, 0xFFFF);
    //GetResolution();
    
    gEnd = false;
    
    if (fVerb>1) {
      std::stringstream s; s << "TDC with base address 0x" << std::hex << baseaddr << " successfully built!";
      PrintInfo(s.str());
    }
  }

  TDCV1x90::~TDCV1x90()
  {
     free(fBuffer);
     fBuffer = NULL;
  }

  uint32_t
  TDCV1x90::GetModel() const
  {
    uint32_t model = 0x0;
    uint16_t data[3];
    TDCV1x90Register addr[3] = {kROMBoard0, kROMBoard1, kROMBoard2};
    try {
      for (int i=0; i<3; i++) {
        ReadRegister(addr[i], &(data[i]));
      }
      model = (((data[2]&0xff) << 16)+((data[1]&0xff) << 8)+(data[0]&0xff));
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: Model is " << std::dec << model;
      PrintInfo(o.str());
    }
    
    return model;
  }

  uint32_t
  TDCV1x90::GetOUI() const
  {
    uint32_t oui = 0x0;
    uint16_t data[3];
    TDCV1x90Register addr[3] = {kROMOui0, kROMOui1, kROMOui2};
    try {
      for (int i=0; i<3; i++) {
        ReadRegister(addr[i], &(data[i]));
      }
      oui = (((data[2]&0xff) << 16)+((data[1]&0xff) << 8)+(data[0]&0xff));
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o;
      o << "Debug: OUI manufacturer number is " << std::dec << oui;
      PrintInfo(o.str());
    }
    
    return oui;
  }

  uint32_t
  TDCV1x90::GetSerialNumber() const
  {
    uint32_t sn = 0x0;
    uint16_t data[2];
    TDCV1x90Register addr[2] = {kROMSerNum0, kROMSerNum1};
    try {
      for (int i=0; i<2; i++) {
        ReadRegister(addr[i], &(data[i]));
      }
      sn = (((data[1]&0xff) << 8)+(data[0]&0xff));
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: Serial number is " << std::dec << sn;
      PrintInfo(o.str());
    }
    
    return sn;
  }

  void
  TDCV1x90::GetFirmwareRevision() const
  {
    //FIXME need to clean up
    uint32_t fr[2] = { 0, 0 };
    uint16_t data;
    try {
      ReadRegister(kFirmwareRev,&data);
      fr[0] = data&0xF;
      fr[1] = (data&0xF0)>>4;
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o;
      o << "Debug: Firmware revision is " << std::dec << fr[1] << "." << fr[0];
      PrintInfo(o.str());
    }
  }

  void
  TDCV1x90::CheckConfiguration() const
  {
    uint32_t oui;
    uint32_t model;

    oui = GetOUI();
    model = GetModel();

    if (oui!=0x0040e6) { // CAEN
      std::ostringstream o; o << "Wrong manufacturer identifier: 0x" << std::hex << oui;
      throw Exception(__PRETTY_FUNCTION__, o.str(), Fatal);
    }
    if ((model!=1190) and (model!=1290)) {
      std::ostringstream o; o << "Wrong model number: model is " << std::dec << model;
      throw Exception(__PRETTY_FUNCTION__, o.str(), Fatal);
    }
    
    /*#ifdef DEBUG
    std::cout << "[VME] <TDC::CheckConfiguration> Debug:" << std::endl;
    std::cout << "       OUI manufacturer number is 0x"
              << std::hex << std::setfill('0') << std::setw(6) << oui << std::endl;
    std::cout << "                  Model number is "
              << std::dec << model << std::endl;
    std::cout << "                 Serial number is "
              << std::dec << GetSerNum() << std::endl;
    #endif*/
  }

  void
  TDCV1x90::SetTestMode(bool en) const
  {
    uint16_t word;
    word = (en) ? TDCV1x90Opcodes::ENABLE_TEST_MODE : TDCV1x90Opcodes::DISABLE_TEST_MODE;

    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, word);
    } catch (Exception& e) { e.Dump(); }

    if (fVerb>1) {
      std::ostringstream o;
      o << "Debug: Test mode enabled? " << en;
      PrintInfo(o.str());
    }
  }

  void
  TDCV1x90::SetPoI(uint16_t word1, uint16_t word2) const
  {
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::WRITE_EN_PATTERN);
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, word1);
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, word2);
    } catch (Exception& e) { e.Dump(); }
    if (fVerb>1) {
      std::ostringstream os;
      os << "Debug: Pattern of inhibit modified:" << "\n\t";
      for (unsigned int i=0; i<16; i++) {
        os << "Channel " << i << ": " << ((word1>>i)&0x1) << "\t"
           << "Channel " << (i+16) << ": " << ((word2>>i)&0x1) << "\n\t";
      }
      PrintInfo(os.str());
    }
  }

  std::map<unsigned short, bool>
  TDCV1x90::GetPoI() const
  {
    uint16_t word1, word2;
    std::map<unsigned short, bool> pattern;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::READ_EN_PATTERN);
      WaitMicro(READ_OK);
      ReadRegister(kMicro, &word1);
      WaitMicro(READ_OK);
      ReadRegister(kMicro, &word2);
    } catch (Exception& e) { e.Dump(); }
    for (unsigned int i=0; i<16; i++) {
      pattern.insert(std::pair<unsigned short, bool>(i, static_cast<bool>((word1>>i)&0x1)));
      pattern.insert(std::pair<unsigned short, bool>(i+16, static_cast<bool>((word2>>i)&0x1)));
    }
    return pattern;
  }

  void
  TDCV1x90::EnableChannel(short channel_id) const
  {
    uint16_t value = TDCV1x90Opcodes::EN_CHANNEL+(channel_id&0xFF);
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, value);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: Channel " << channel_id << " enabled";
      PrintInfo(o.str());
    }
  }

  void
  TDCV1x90::DisableChannel(short channel_id) const
  {
    uint16_t value = TDCV1x90Opcodes::DIS_CHANNEL+(channel_id&0xFF);
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, value);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: Channel " << channel_id << " disabled";
      PrintInfo(o.str());
    }
  }

  void TDCV1x90::SetLSBTraileadEdge(trailead_edge_lsb conf) const
  {
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::SET_TR_LEAD_LSB);
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, static_cast<uint16_t>(conf));
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::stringstream o; o << "Debug: ";
      switch(conf){
        case r800ps: o << "800ps"; break;
        case r200ps: o << "200ps"; break;
        case r100ps: o << "100ps"; break;
        case r25ps: o << "25ps"; break;
      }
      PrintInfo(o.str());
    }
  }

  void
  TDCV1x90::SetGlobalOffset(const GlobalOffset& offs) const
  {
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::SET_GLOB_OFFS);
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, offs.coarse);
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, offs.fine);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o;
      o << "Debug: " << std::endl
        << "\tcoarse counter offset: " << offs.coarse << std::endl
        << "\t  fine counter offset: " << offs.fine;
      PrintInfo(o.str());
    }
  }

  GlobalOffset
  TDCV1x90::GetGlobalOffset() const
  {
    GlobalOffset ret;
    uint16_t data[2];
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::READ_GLOB_OFFS);
      int i;
      for(i=0;i<2;i++){
        WaitMicro(READ_OK);
        ReadRegister(kMicro, &(data[i]));
      }
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o;
      o << "Debug: " << std::endl
        << "\tcoarse counter offset: " << data[0] << std::endl
        << "\t  fine counter offset: " << data[1];
      PrintInfo(o.str());
    }
    ret.fine = data[1];
    ret.coarse = data[0];
    return ret;
  }

  void
  TDCV1x90::SetRCAdjust(int tdc, uint16_t value) const
  {
    //FIXME find a better way to insert value for 12 RCs
    uint16_t word = value;
    uint16_t opcode = TDCV1x90Opcodes::SET_RC_ADJ+(tdc&0x3);
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, opcode);
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, word);
    } catch (Exception& e) { e.Dump(); }
    
    /*opcode = TDCV1x90Opcodes::SAVE_RC_ADJ;
    WaitMicro(WRITE_OK);
    WriteRegister(kMicro, opcode);
    WaitMicro(WRITE_OK);
    WriteRegister(kMicro, word); */
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: TDC " << tdc << ", value " << value;
      PrintInfo(o.str());
    }
  }

  uint16_t
  TDCV1x90::GetRCAdjust(int tdc) const
  {
    uint16_t opcode = TDCV1x90Opcodes::READ_RC_ADJ+(tdc&0x3);
    uint16_t data;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, opcode);
      WaitMicro(READ_OK);
      ReadRegister(kMicro, &data);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: value for TDC " << tdc << std::endl;
      for(int i=0; i<12; i++) {
        o << "\t  bit " << std::setw(2) << i << ": ";
        char bit = ((data>>i)&0x1);
        switch(bit) {
          case 0: o << "contact open"; break;
          case 1: o << "contact closed"; break;
        }
        if (i<11) o << std::endl;
      }
      PrintInfo(o.str());
    }
    return data;
  }

  void
  TDCV1x90::SetDetectionMode(const DetectionMode& mode)
  {
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::SET_DETECTION);
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, static_cast<uint16_t>(mode));
    } catch (Exception& e) { e.Dump(); }
    fDetectionMode = mode;
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: ";
      switch(mode){
        case PAIR: o << "pair mode"; break;
        case OTRAILING: o << "only trailing"; break;
        case OLEADING: o << "only leading"; break;
        case TRAILEAD: o << "trailing and leading"; break;
      }
      PrintInfo(o.str());
    }
  }

  void
  TDCV1x90::ReadDetectionMode()
  {
    uint16_t data;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::READ_DETECTION);
      WaitMicro(READ_OK);
      ReadRegister(kMicro, &data);
    } catch (Exception& e) { e.Dump(); }
    fDetectionMode = static_cast<DetectionMode>(data&0x3);
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: ";
      switch(fDetectionMode){
        case PAIR: o << "pair mode"; break;
        case OTRAILING: o << "only trailing"; break;
        case OLEADING: o << "only leading"; break;
        case TRAILEAD: o << "trailing and leading"; break;
      }
      PrintInfo(o.str());
    }
  }

  void
  TDCV1x90::SetWindowWidth(const uint16_t& width)
  {
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::SET_WIN_WIDTH);
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, width);
    } catch (Exception& e) { e.Dump(); return; }
    fWindowWidth = width;
  }

  void
  TDCV1x90::SetWindowOffset(const int16_t& offs) const
  {
    //FIXME warning at sign bit
    uint16_t data = offs;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::SET_WIN_OFFS);
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, data);
    } catch (Exception& e) { e.Dump(); }
  }
    
  uint16_t
  TDCV1x90::GetTriggerConfiguration(const trig_conf& type) const
  {
    uint16_t buff[5];
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::READ_TRG_CONF);
      for (int i=0; i<5; i++) {
        WaitMicro(READ_OK);
        ReadRegister(kMicro,&(buff[i]));
      }
    } catch (Exception& e) { e.Dump(); }
    return buff[type];
  }

  uint16_t
  TDCV1x90::GetResolution() const
  {
    uint16_t data;
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::READ_RES);
      WaitMicro(READ_OK);
      ReadRegister(kMicro, &data);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      const char* pair_lead_res[] = {
        "100ps", "200ps", "400ps", "800ps", "1.6ns", "3.12ns", "6.25ns", "12.5ns"
      };
      const char* pair_width_res[] = {
        "100ps", "200ps", "400ps", "800ps", "1.6ns", "3.2ns", "6.25ns", "12.5ns",
        "25ns", "50ns", "100ns", "200ns", "400ns", "800ns", "invalid","invalid"
      };
      const char* trailead_edge_res[] = { "800ps", "200ps", "100ps", "25ps" };
      std::ostringstream os; os << " Debug: ";
      switch(fDetectionMode) {
        case PAIR: 
          os << "(pair mode) leading edge res.: " << pair_lead_res[data&0x7]
             << ", pulse width res.: " << pair_width_res[(data&0xF00)>>8];
          break;
        case OLEADING:
        case OTRAILING:
        case TRAILEAD:
          os << "(l/t mode) leading/trailing edge res.: " << trailead_edge_res[data&0x3];
          break;
      }
      PrintInfo(os.str());
    }
    return data;
  }

  void
  TDCV1x90::SetPairModeResolution(int lead_time_res, int pulse_width_res) const
  {
    uint16_t data = lead_time_res+0x100*pulse_width_res;
    /*(data&0x7)=lead_time_res;
    (data&0xf00)=pulse_width_res;*/
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::SET_PAIR_RES);
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, data);
    } catch (Exception& e) { e.Dump(); }
  }


  void
  TDCV1x90::SetAcquisitionMode(const AcquisitionMode& mode)
  {
    try {
      switch(mode){
        case CONT_STORAGE: SetContinuousStorage(); break;
        case TRIG_MATCH:   SetTriggerMatching();   break;
        default:
          throw Exception(__PRETTY_FUNCTION__, "Wrong acquisition mode", Fatal);
      }
    } catch (Exception& e) { throw e; }
  }

  void
  TDCV1x90::SetTriggerMatching()
  {
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::TRG_MATCH);
      WaitMicro(WRITE_OK);
      if (GetAcquisitionMode()!=TRIG_MATCH)
        throw Exception(__PRETTY_FUNCTION__, "Error while setting the acquisition mode to trigger matching!", Fatal);

    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      PrintInfo("Debug: trigger matching mode");
    }
  }

  void
  TDCV1x90::SetContinuousStorage()
  {
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::CONT_STOR);
      WaitMicro(WRITE_OK);
      if (GetAcquisitionMode()!=CONT_STORAGE)
        throw Exception(__PRETTY_FUNCTION__, "Error while setting the acquisition mode to continuous storage!", Fatal);

    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      PrintInfo("Debug: continuous storage mode");
    }
  }

  void
  TDCV1x90::ReadAcquisitionMode()
  {
    uint16_t data;

    /*uint32_t addr = fBaseAddr+0x1002; // Status register
    std::cout << "ReadCycle response: " << std::dec << CAENVME_ReadCycle(fHandle,addr,&data,am,cvD16) << std::endl;
    std::cout << "isTriggerMatching: value: " << ((data>>3)&0x1) << std::endl;*/

    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::READ_ACQ_MOD);
      WaitMicro(READ_OK);
      ReadRegister(kMicro, &data);
    } catch (Exception& e) { e.Dump(); }
    if (fVerb>1) {  
      std::ostringstream o; o << "Debug: value: " << data << " (";
      switch (data) {
        case 0: o << "continuous storage"; break;
        case 1: o << "trigger matching"; break;
        default: o << "wrong answer!"; break;
      }
      o << ")";
      PrintInfo(o.str());
    }
    switch (data) {
      case 0: fAcquisitionMode = CONT_STORAGE; break;
      case 1: fAcquisitionMode = TRIG_MATCH; break;
      default: 
        throw Exception(__PRETTY_FUNCTION__, "Invalid acquisition mode read from board!", Fatal);
    }
  }

  bool
  TDCV1x90::SoftwareReset() const
  {
    try { WriteRegister(kModuleReset, static_cast<uint16_t>(0x0)); } catch (Exception& e) { e.Dump(); }
    return true;
  }

  bool
  TDCV1x90::SoftwareClear() const
  {
    try { WriteRegister(kSoftwareClear, static_cast<uint16_t>(0x0)); } catch (Exception& e) { e.Dump(); }
    return true;
  }

  //bool
  //TDCV1x90::IsEventFIFOReady()
  //{
    //std::cout << "[VME] <TDC::ifEventFIFOReady> Debug: is FIFO enabled: "
    //          << GetControlRegister(EVENT_FIFO_ENABLE) << std::endl;
    //SetFIFOSize(7); //FIXME
    //ReadFIFOSize();
    /*ReadRegister(kEventFIFOStatusRegister,&data);
    std::cout << "[VME] <TDC::ifEventFIFOReady> Debug: data: " << data << std::endl;
    std::cout << "                DATA_READY: " << (data&1) << std::endl;
    std::cout << "                      FULL: " << ((data&2)>>1) << std::endl;
    ReadRegister(kEventFIFOStoredRegister,&data2);
    std::cout << "[VME] <TDC::ifEventFIFOReady> Debug: data2: " << ((data2&0x7ff)>>11) << std::endl;*/
  //}

  void
  TDCV1x90::SetChannelDeadTime(unsigned short dt) const
  {
    uint16_t word = (dt&0x3);
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::SET_DEAD_TIME);
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, word);
    } catch (Exception& e) { e.Dump(); }
    if (fVerb>1) {
      std::ostringstream os;
      os << "Debug: Channel dead time set to: ";
      switch (word) {
        case 0x0: os << "5 ns"; break;
        case 0x1: os << "10 ns"; break;
        case 0x2: os << "30 ns"; break;
        case 0x3: os << "100 ns"; break;
      }
      PrintInfo(os.str());
    }
  }

  unsigned short
  TDCV1x90::GetChannelDeadTime() const
  {
    uint16_t dt;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::READ_DEAD_TIME);
      WaitMicro(READ_OK);
      ReadRegister(kMicro, &dt);
    } catch (Exception& e) { e.Dump(); }
    return static_cast<unsigned short>(dt&0x3);
  }

  void
  TDCV1x90::SetFIFOSize(const uint16_t& size) const
  {
    uint16_t word;
    switch(size) {
      case 2:   word=0x0; break;
      case 4:   word=0x1; break;
      case 8:   word=0x2; break;
      case 16:  word=0x3; break;
      case 32:  word=0x4; break;
      case 64:  word=0x5; break;
      case 128: word=0x6; break;
      case 256: word=0x7; break;
      default:
        throw Exception(__PRETTY_FUNCTION__, "Trying to set a wrong FIFO size.", JustWarning);
    }
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::SET_FIFO_SIZE);
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, word);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: Setting the FIFO size to: " << size << " (" << word << ")";
      PrintInfo(o.str());
    }
  }

  uint16_t
  TDCV1x90::GetFIFOSize() const
  {
    uint16_t data;
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::READ_FIFO_SIZE);
      WaitMicro(READ_OK);
      ReadRegister(kMicro, &data);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: READ_FIFO_SIZE: " << std::dec << (1<<(data+1));
      PrintInfo(o.str());
    }
    return (1<<(data+1));
  }

  void
  TDCV1x90::SetTDCEncapsulation(bool mode) const
  {
    uint16_t opcode;
    if (mode) opcode = TDCV1x90Opcodes::EN_HEAD_TRAILER;
    else      opcode = TDCV1x90Opcodes::DIS_HEAD_TRAILER;
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, opcode);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: TDC encapsulation enabled? " << mode;
      PrintInfo(o.str());
    }
  }

  bool
  TDCV1x90::GetTDCEncapsulation() const
  {
    uint16_t enc;
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::READ_HEAD_TRAILER);
      WaitMicro(READ_OK);
      ReadRegister(kMicro, &enc);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: READ_HEAD_TRAILER: " << enc;
      PrintInfo(o.str());
    }
    return static_cast<bool>(enc&0x1);
  }

  uint32_t
  TDCV1x90::GetEventCounter() const
  {
    uint32_t value;
    try { ReadRegister(kEventCounter, &value); } catch (Exception& e) { e.Dump(); }
    return value&0xFFFFFFFF;
  }

  uint16_t
  TDCV1x90::GetEventStored() const
  {
    uint16_t value;
    try { ReadRegister(kEventStored, &value); } catch (Exception& e) { e.Dump(); }
    return value&0xFFFF;
  }

  void
  TDCV1x90::SetErrorMarks(bool mode)
  {
    uint16_t opcode;
    if (mode) opcode = TDCV1x90Opcodes::EN_ERROR_MARK;
    else      opcode = TDCV1x90Opcodes::DIS_ERROR_MARK;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, opcode);
    } catch (Exception& e) { e.Dump(); return; }
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: Enabled? " << mode;
      PrintInfo(o.str());
    }
    fErrorMarks = mode;
  }

  void
  TDCV1x90::SetBLTEventNumberRegister(const uint16_t& value) const
  {
    try { WriteRegister(kBLTEventNumber, value); } catch (Exception& e) { e.Dump(); }
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: value: " << value;
      PrintInfo(o.str());
    }
  }

  uint16_t
  TDCV1x90::GetBLTEventNumberRegister() const
  {
    uint16_t value;
    try { ReadRegister(kBLTEventNumber, &value); } catch (Exception& e) { e.Dump(); }
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: value: " << value;
      PrintInfo(o.str());
    }
    return value;
  }

  void
  TDCV1x90::SetDLLClock(const DLLMode& dll) const
  {
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, TDCV1x90Opcodes::SET_DLL_CLOCK);
      WaitMicro(WRITE_OK);
      WriteRegister(kMicro, static_cast<uint16_t>(dll));
    } catch (Exception& e) { e.Dump(); return; }
  }
    
  void
  TDCV1x90::SetStatus(const TDCV1x90Status& status) const
  {
    try { WriteRegister(kStatus, status.GetValue()); } catch (Exception& e) { e.Dump(); }
  }

  TDCV1x90Status
  TDCV1x90::GetStatus() const
  {
    uint16_t value;
    try { ReadRegister(kStatus, &value); } catch (Exception& e) { e.Dump(); }
    return TDCV1x90Status(value&0xFFFF);
  }

  void
  TDCV1x90::SetControl(const TDCV1x90Control& control) const
  {
    try { WriteRegister(kControl, control.GetValue()); } catch (Exception& e) { e.Dump(); }
  }

  TDCV1x90Control
  TDCV1x90::GetControl() const
  {
    uint16_t value;
    try { ReadRegister(kControl, &value); } catch (Exception& e) { e.Dump(); }
    return TDCV1x90Control(value&0xFFFF);
  }

  TDCEventCollection
  TDCV1x90::FetchEvents()
  {
    if (gEnd)
      throw Exception(__PRETTY_FUNCTION__, "Abort state detected... quitting", JustWarning, TDC_ACQ_STOP);
    TDCEventCollection ec;
    // Start Readout (check if BERR is set to 0)
    // New words are transmitted until the global TRAILER
    
    memset(fBuffer, 0, sizeof(uint32_t));
    
    int count=0;
    const int blts = 2048;
    bool finished;
    //int value, channel, trailing, width; // for continuous storage mode
    std::ostringstream o;

    CVErrorCodes ret;
    ret = CAENVME_BLTReadCycle(fHandle, fBaseAddr+kOutputBuffer, (char*)fBuffer, blts, cvA32_U_BLT, cvD32, &count);
    finished = ((ret==cvSuccess)||(ret==cvBusError)||(ret==cvCommError)); //FIXME investigate...
    if (finished && gEnd) {
      if (fVerb>1) {
        PrintInfo("Debug: Exit requested!");
      }
      //exit(0);
      throw Exception(__PRETTY_FUNCTION__, "Abort state detected... quitting", JustWarning, TDC_ACQ_STOP);
    }
    switch (fAcquisitionMode) {
    case TRIG_MATCH:
      for (int i=0; i<count/4; i++) { // FIXME need to use the knowledge of the TDCEvent behaviour there...
        //if (fBuffer[i]==0) continue;
        TDCEvent ev(fBuffer[i]);
        if (ev.GetType()==TDCEvent::Filler) continue; // Filter out filler data
        ec.push_back(ev);
      }
      return ec;

    case CONT_STORAGE:
      for (int i=0; i<count; i++) {
        TDCEvent ev(fBuffer[i]);
        ec.push_back(ev);
        //trailing = ev.IsTrailing();
        //value = (trailing) ? ev.GetTrailingTime() : ev.GetLeadingTime();
        //channel = ev.GetChannelId();
        /*std::cout << std::dec
                  << "event " << i << "\t"
                  << "channel " << channel << "\t";
        switch(fDetectionMode) {
          case PAIR:
            width = (fBuffer[i]&0x7F000)>>12;
            value = fBuffer[i]&0xFFF;
            std::cout << "width " << std::hex << width << "\t\t"
                      << "value " << std::dec << value;
          break;
        case OTRAILING:
        case OLEADING:
          std::cout << std::dec
                    << "value " << value << "\t"
                    << "trailing? " << trailing;
          break;
        case TRAILEAD:
          std::cout << std::dec 
                    << "value " << value << "\t"
                    << "trailing? " << trailing;
          break;
        default:
          o.str(""); o << "Wrong detection mode: " << fDetectionMode;
          throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
        }
        //std::cout << std::endl;*/
      }
      return ec;
      
    default:
      o.str(""); o << "Wrong acquisition mode: " << fAcquisitionMode;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
    
  }

  void
  TDCV1x90::abort()
  {
    if (fVerb>1) {
      PrintInfo("Debug: received abort signal");
    }
    // Raise flag
    gEnd = true;
  }

  bool
  TDCV1x90::WaitMicro(micro_handshake mode) const
  {
    uint16_t data;
    bool status = false;
    while (!status) {
      ReadRegister(kMicroHandshake, &data);
      switch(mode){
        case WRITE_OK: status = static_cast<bool>(data&0x1); break;
        case READ_OK:  status = static_cast<bool>((data>>1)&0x1); break;
        default: return false;
      }
    }
    return status;
  }
}
