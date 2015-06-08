#include "VME_TDCV1x90.h"

namespace VME
{
  TDCV1x90::TDCV1x90(int32_t bhandle,uint32_t baseaddr, acq_mode acqm, det_mode detm) :
    fBaseAddr(baseaddr), fHandle(bhandle), fDetMode(detm),
    am(cvA32_U_DATA), am_blt(cvA32_U_BLT)
  {
    //event_nb = 0;
    //event_max = 1024;

    fBuffer = (uint32_t *)malloc(16*1024*1024); // 16Mb of buffer!
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
    SetAcquisitionMode(acqm);
    
    SetDetection(TRAILEAD);
    SetLSBTraileadEdge(r25ps);
    SetRCAdjust(0,0);
    SetRCAdjust(1,0);
    SetGlobalOffset(0x0,0x0); // coarse and fine set
    ReadGlobalOffset();
    SetBLTEventNumberRegister(1); // FIXME find good value!
    SetTDCEncapsulation(true);
    SetTDCErrorMarks(true);
    SetETTT(true);
    SetWindowWidth(2045);
    SetWindowOffset(-2050);
    //SetPairModeResolution(0,0x4);
    //ReadResolution(detect);
    
    gEnd = false;
    
    const char* c_pair_lead_res[] = {
      "100ps", "200ps", "400ps", "800ps", "1.6ns", "3.12ns", "6.25ns", "12.5ns"
    };
    const char* c_pair_width_res[] = {
      "100ps", "200ps", "400ps", "800ps", "1.6ns", "3.2ns", "6.25ns", "12.5ns",
      "25ns", "50ns", "100ns", "200ns", "400ns", "800ns", "invalid","invalid"
    };
    const char* c_trailead_edge_res[] = { "800ps", "200ps", "100ps", "25ps" };
    for (int i=0; i<8; i++) pair_lead_res[i] = c_pair_lead_res[i];
    for (int i=0; i<16; i++) pair_width_res[i] = c_pair_width_res[i];
    for (int i=0; i<4; i++) trailead_edge_res[i] = c_trailead_edge_res[i];
  }

  TDCV1x90::~TDCV1x90()
  {
     free(fBuffer);
     fBuffer = NULL;
  }

  uint32_t
  TDCV1x90::GetModel()
  {
    uint32_t model = 0x0;
    uint16_t data[3];
    mod_reg addr[3] = {ROMBoard0, ROMBoard1, ROMBoard2};
    try {
      for (int i=0; i<3; i++) {
        ReadRegister(addr[i],&(data[i]));
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
  TDCV1x90::GetOUI()
  {
    uint32_t oui = 0x0;
    uint16_t data[3];
    mod_reg addr[3] = {ROMOui0, ROMOui1, ROMOui2};
    try {
      for (int i=0; i<3; i++) {
        ReadRegister(addr[i],&(data[i]));
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
  TDCV1x90::GetSerialNumber()
  {
    uint32_t sn = 0x0;
    uint16_t data[2];
    mod_reg addr[2] = {ROMSerNum0, ROMSerNum1};
    try {
      for (int i=0; i<2; i++) {
        ReadRegister(addr[i],&(data[i]));
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
  TDCV1x90::GetFirmwareRev()
  {
    //FIXME need to clean up
    uint32_t fr[2];
    uint16_t data;
    try {
      ReadRegister(FirmwareRev,&data);
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
  TDCV1x90::CheckConfiguration()
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
  TDCV1x90::SetPoI(uint16_t word)
  {
    // ...
  }

  void
  TDCV1x90::EnableChannel(short channel_id)
  {
    uint16_t value = TDCV1x90Opcodes::EN_CHANNEL+(channel_id&0xFF);
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(Micro, &value);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: Channel " << channel_id << " enabled";
      PrintInfo(o.str());
    }
  }

  void
  TDCV1x90::DisableChannel(short channel_id)
  {
    uint16_t value = TDCV1x90Opcodes::DIS_CHANNEL+(channel_id&0xFF);
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(Micro, &value);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: Channel " << channel_id << " disabled";
      PrintInfo(o.str());
    }
  }

  void TDCV1x90::SetLSBTraileadEdge(trailead_edge_lsb conf)
  {
    uint16_t word = conf;
    uint16_t value = TDCV1x90Opcodes::SET_TR_LEAD_LSB;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(Micro, &value);
      WaitMicro(WRITE_OK);
      WriteRegister(Micro, &word);
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
  TDCV1x90::SetGlobalOffset(uint16_t word1,uint16_t word2)
  {
    uint16_t opcode = TDCV1x90Opcodes::SET_GLOB_OFFS;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&opcode);
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&word1);
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&word2);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o;
      o << "Debug: " << std::endl
        << "\tcoarse counter offset: " << word1 << std::endl
        << "\t  fine counter offset: " << word2 << std::endl;
      PrintInfo(o.str());
    }
  }

  glob_offs
  TDCV1x90::ReadGlobalOffset()
  {
    uint16_t opcode = TDCV1x90Opcodes::READ_GLOB_OFFS;
    uint16_t data[2];
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&opcode);
      int i;
      for(i=0;i<2;i++){
        WaitMicro(READ_OK);
        ReadRegister(Micro,&(data[i]));
      }
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o;
      o << "Debug: " << std::endl
        << "\tcoarse counter offset: " << data[0] << std::endl
        << "\t  fine counter offset: " << data[1] << std::endl;
      PrintInfo(o.str());
    }
    glob_offs ret;
    ret.fine = data[1];
    ret.coarse = data[0];
    return ret;
  }

  void
  TDCV1x90::SetRCAdjust(int tdc, uint16_t value)
  {
    //FIXME find a better way to insert value for 12 RCs
    uint16_t word = value;
    uint16_t opcode = TDCV1x90Opcodes::SET_RC_ADJ+(tdc&0x3);
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&opcode);
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&word);
    } catch (Exception& e) { e.Dump(); }
    
    /*opcode = TDCV1x90Opcodes::SAVE_RC_ADJ;
    WaitMicro(WRITE_OK);
    WriteRegister(Micro,&opcode);
    WaitMicro(WRITE_OK);
    WriteRegister(Micro,&word); */
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: TDC " << tdc << ", value " << value;
      PrintInfo(o.str());
    }
  }

  uint16_t
  TDCV1x90::ReadRCAdjust(int tdc)
  {
    uint16_t opcode = TDCV1x90Opcodes::READ_RC_ADJ+(tdc&0x3);
    uint16_t data;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&opcode);
      WaitMicro(READ_OK);
      ReadRegister(Micro,&data);
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
  TDCV1x90::SetDetection(det_mode mode)
  {
    uint16_t word = mode;
    uint16_t value = TDCV1x90Opcodes::SET_DETECTION;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&value);
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&word);
    } catch (Exception& e) { e.Dump(); }
    
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

  det_mode
  TDCV1x90::ReadDetection()
  {
    uint16_t value = TDCV1x90Opcodes::READ_DETECTION;
    uint16_t data;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&value);
      WaitMicro(READ_OK);
      ReadRegister(Micro,&data);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: ";
      switch(data){
        case PAIR: o << "pair mode"; break;
        case OTRAILING: o << "only trailing"; break;
        case OLEADING: o << "only leading"; break;
        case TRAILEAD: o << "trailing and leading"; break;
      }
      PrintInfo(o.str());
    }
    return (det_mode)data;
  }

  void
  TDCV1x90::SetWindowWidth(uint16_t width)
  {
    uint16_t word = width;
    uint16_t value = TDCV1x90Opcodes::SET_WIN_WIDTH;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&value);
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&word);
    } catch (Exception& e) { e.Dump(); }
  }

  void
  TDCV1x90::SetWindowOffset(int16_t offs)
  {
    //FIXME warning at sign bit
    uint16_t word = offs;
    uint16_t value = TDCV1x90Opcodes::SET_WIN_OFFS;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&value);
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&word);
    } catch (Exception& e) { e.Dump(); }
  }
    
  uint16_t
  TDCV1x90::ReadTrigConf(trig_conf type)
  {
    uint16_t value = TDCV1x90Opcodes::READ_TRG_CONF;
    uint16_t buff[5];
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&value);
      for (int i=0; i<5; i++) {
        WaitMicro(READ_OK);
        ReadRegister(Micro,&(buff[i]));
      }
    } catch (Exception& e) { e.Dump(); }
    return buff[type];
  }

  void
  TDCV1x90::ReadResolution(det_mode det)
  {
    uint16_t value = TDCV1x90Opcodes::READ_RES;
    uint16_t data;
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&value);
      WaitMicro(READ_OK);
      ReadRegister(Micro,&data);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::cout << __PRETTY_FUNCTION__ << " Debug: ";
      switch(det) {
        case PAIR: 
          std::cout << "(pair mode) leading edge res.: " << pair_lead_res[data&0x7]
                    << ", pulse width res.: " << pair_width_res[(data&0xF00)>>8]
                    << std::endl;
          break;
        case OLEADING:
        case OTRAILING:
        case TRAILEAD:
          std::cout << "(l/t mode) leading/trailing edge res.: "
                    << trailead_edge_res[data&0x3] << std::endl;
          break;
      }
    }
  }

  void
  TDCV1x90::SetPairModeResolution(int lead_time_res, int pulse_width_res)
  {
    uint16_t value = TDCV1x90Opcodes::SET_PAIR_RES;
    uint16_t data = 0;
    data = lead_time_res+0x100*pulse_width_res;
    /*(data&0x7)=lead_time_res;
    (data&0xf00)=pulse_width_res;*/
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&value);
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&data);
    } catch (Exception& e) { e.Dump(); }
  }


  void
  TDCV1x90::SetAcquisitionMode(acq_mode mode)
  {
    acqm = mode;
    switch(mode){
      case CONT_STORAGE:
        if (!SetContinuousStorage())
          throw Exception(__PRETTY_FUNCTION__, "Error while entering the continuous storage mode", Fatal);
        break;
      case TRIG_MATCH:
        if (!SetTriggerMatching())
          throw Exception(__PRETTY_FUNCTION__, "Error while entering the trigger matching mode", Fatal);
        break;
      default:
        throw Exception(__PRETTY_FUNCTION__, "Wrong acquisition mode", Fatal);
    }
  }

  bool
  TDCV1x90::SetTriggerMatching()
  {
    uint16_t value = TDCV1x90Opcodes::TRG_MATCH;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&value);
      WaitMicro(WRITE_OK);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      PrintInfo("Debug: trigger matching mode");
    }
    return true;
  }

  acq_mode
  TDCV1x90::GetAcquisitionMode() const
  {
    uint16_t data;

    /*uint32_t addr = fBaseAddr+0x1002; // Status register
    std::cout << "ReadCycle response: " << std::dec << CAENVME_ReadCycle(fHandle,addr,&data,am,cvD16) << std::endl;
    std::cout << "isTriggerMatching: value: " << ((data>>3)&0x1) << std::endl;*/

    uint16_t value = TDCV1x90Opcodes::READ_ACQ_MOD;
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&value);
      WaitMicro(READ_OK);
      ReadRegister(Micro,&data);
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
      case 0: return CONT_STORAGE;
      case 1: return TRIG_MATCH;
      default: return (acq_mode)(-1);
    }
  }

  bool
  TDCV1x90::SetContinuousStorage()
  {
    uint16_t value = TDCV1x90Opcodes::CONT_STOR;
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&value);
      WaitMicro(WRITE_OK); } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      PrintInfo("Debug: continuous storage mode");
    }
    return true;
  }

  bool
  TDCV1x90::SoftwareReset()
  {
    uint16_t value = 0x0;
    try { WriteRegister(ModuleReset,&value); } catch (Exception& e) { e.Dump(); }
    return true;
  }

  bool
  TDCV1x90::SoftwareClear()
  {
    uint16_t value = 0x0;
    try { WriteRegister(kSoftwareClear,&value); } catch (Exception& e) { e.Dump(); }
    return true;
  }

  bool
  TDCV1x90::GetStatusRegister(stat_reg bit)
  {
    uint16_t data;
    try { ReadRegister(Status,&data); } catch (Exception& e) { e.Dump(); }
    return ((data&(1<<bit))>>bit);
  }

  void
  TDCV1x90::SetStatusRegister(stat_reg reg, bool value)
  {
    bool act = GetStatusRegister(reg);
    if (act==value) return;
    
    uint16_t buff;
    try { 
      ReadRegister(Status,&buff);
      if (value) buff += (1<<reg);
      else       buff -= (1<<reg);
      WriteRegister(Status,&buff);
    } catch (Exception& e) { e.Dump(); }
  }

  bool
  TDCV1x90::GetCtlRegister(ctl_reg bit)
  {
    uint16_t data;
    try { ReadRegister(Control,&data); } catch (Exception& e) { e.Dump(); }
    return ((data&(1<<bit))>>bit);
  }

  void
  TDCV1x90::SetCtlRegister(ctl_reg reg, bool value)
  {
    bool act = GetCtlRegister(reg);
    if (act==value) return;
    
    uint16_t buff;
    try {
      ReadRegister(Control,&buff);
      if (value) buff += (1<<reg);
      else       buff -= (1<<reg);
      WriteRegister(Control,&buff);
    } catch (Exception& e) { e.Dump(); }
  }

  //bool
  //TDCV1x90::IsEventFIFOReady()
  //{
    //std::cout << "[VME] <TDC::ifEventFIFOReady> Debug: is FIFO enabled: "
    //          << GetCtlRegister(EVENT_FIFO_ENABLE) << std::endl;
    //SetFIFOSize(7); //FIXME
    //ReadFIFOSize();
    /*ReadRegister(EventFIFOStatusRegister,&data);
    std::cout << "[VME] <TDC::ifEventFIFOReady> Debug: data: " << data << std::endl;
    std::cout << "                DATA_READY: " << (data&1) << std::endl;
    std::cout << "                      FULL: " << ((data&2)>>1) << std::endl;
    ReadRegister(EventFIFOStoredRegister,&data2);
    std::cout << "[VME] <TDC::ifEventFIFOReady> Debug: data2: " << ((data2&0x7ff)>>11) << std::endl;*/
  //}

  void
  TDCV1x90::SetFIFOSize(uint16_t size)
  {
    //std::cout << "size: " << (int)(std::log(size/2)/std::log(2)) << std::endl;
    //FIXME! do some crappy math
    uint16_t word;
    switch(size) {
      case 2: word=0; break;
      case 4: word=1; break;
      case 8: word=2; break;
      case 16: word=3; break;
      case 32: word=4; break;
      case 64: word=5; break;
      case 128: word=6; break;
      case 256: word=7; break;
      default: exit(0);
    }
    uint16_t opcode = TDCV1x90Opcodes::SET_FIFO_SIZE;
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&opcode);
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&word);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: WRITE_FIFO_SIZE: " << word;
      PrintInfo(o.str());
    }
  }

  void
  TDCV1x90::ReadFIFOSize()
  {
    uint16_t word = TDCV1x90Opcodes::READ_FIFO_SIZE;
    uint16_t data;
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&word);
      WaitMicro(READ_OK);
      ReadRegister(Micro,&data);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: READ_FIFO_SIZE: " << std::dec << (1<<(data+1));
      PrintInfo(o.str());
    }
  }

  void
  TDCV1x90::SetTDCEncapsulation(bool mode)
  {
    uint16_t opcode;
    if (mode) opcode = TDCV1x90Opcodes::EN_HEAD_TRAILER;
    else      opcode = TDCV1x90Opcodes::DIS_HEAD_TRAILER;
    outBufTDCHeadTrail = mode;
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&opcode);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: Enabled? " << mode; PrintInfo(o.str());
    }
  }

  bool
  TDCV1x90::GetTDCEncapsulation()
  {
    uint16_t opcode = TDCV1x90Opcodes::READ_HEAD_TRAILER;
    uint16_t enc;
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&opcode);
      WaitMicro(READ_OK);
      ReadRegister(Micro,&enc);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: READ_HEAD_TRAILER: " << enc;
      PrintInfo(o.str());
    }
    return enc;
  }

  uint32_t
  TDCV1x90::GetEventCounter()
  {
    uint32_t value;
    try { ReadRegister(EventCounter,&value); } catch (Exception& e) { e.Dump(); }
    return value;
  }

  uint16_t
  TDCV1x90::GetEventStored()
  {
    uint16_t value;
    try { ReadRegister(EventStored,&value); } catch (Exception& e) { e.Dump(); }
    return value;
  }

  void
  TDCV1x90::SetTDCErrorMarks(bool mode)
  {
    uint16_t opcode;
    if (mode) opcode = TDCV1x90Opcodes::EN_ERROR_MARK;
    else      opcode = TDCV1x90Opcodes::DIS_ERROR_MARK;
    outBufTDCErr = mode;
    WaitMicro(WRITE_OK);
    WriteRegister(Micro,&opcode);
    
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: Enabled? " << mode; PrintInfo(o.str());
    }
  }

  /*bool TDCV1x90::GetTDCErrorMarks() {
    uint16_t opcode = TDCV1x90Opcodes::READ_HEAD_TRAILER;
  }*/

  void
  TDCV1x90::SetBLTEventNumberRegister(uint16_t value)
  {
    try { WriteRegister(BLTEventNumber,&value); } catch (Exception& e) { e.Dump(); }
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: value: " << value; PrintInfo(o.str());
    }
  }

  uint16_t
  TDCV1x90::GetBLTEventNumberRegister()
  {
    uint16_t value;
    try { ReadRegister(BLTEventNumber,&value); } catch (Exception& e) { e.Dump(); }
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: value: " << value; PrintInfo(o.str());
    }
    return value;
  }
    
  void
  TDCV1x90::SetETTT(bool mode)
  {
    SetCtlRegister(EXTENDED_TRIGGER_TIME_TAG_ENABLE,mode);
    outBufTDCTTT = mode;
    if (fVerb>1) {
      std::ostringstream o; o << "Debug: Enabled? " << mode; PrintInfo(o.str());
    }
  }

  bool
  TDCV1x90::GetETTT()
  {
    return GetCtlRegister(EXTENDED_TRIGGER_TIME_TAG_ENABLE);
  }

  TDCEventCollection
  TDCV1x90::FetchEvents()
  {
    if (gEnd)
      throw Exception(__PRETTY_FUNCTION__, "Abort state detected... quitting", JustWarning, TDC_ACQ_STOP);
    TDCEventCollection ec;
    // Start Readout (check if BERR is set to 0)
    // Nw words are transmitted until the global TRAILER
    // Move file!
    
    memset(fBuffer, 0, sizeof(uint32_t));
    
    int count=0;
    int blts = 1024;
    bool finished;
    int i;
    int value, channel, trailing, width; // for continuous storage mode
    std::ostringstream o;

    CVErrorCodes ret;
    ret = CAENVME_BLTReadCycle(fHandle, fBaseAddr+0x0000, (char *)fBuffer, blts, am_blt, cvD32, &count);
    finished = ((ret==cvSuccess)||(ret==cvBusError)||(ret==cvCommError)); //FIXME investigate...
    if (finished && gEnd) {
      if (fVerb>1) {
        PrintInfo("Debug: Exit requested!");
      }
      //exit(0);
      throw Exception(__PRETTY_FUNCTION__, "Abort state detected... quitting", JustWarning, TDC_ACQ_STOP);
    }
    switch (acqm) {
    case TRIG_MATCH:
      for (i=0; i<count; i++) { // FIXME need to use the knowledge of the TDCEvent behaviour there...
        TDCEvent ev(fBuffer[i]);
        if (ev.GetType()==TDCEvent::Filler) continue; // Filter out filler data
        ec.push_back(ev);
      }
      return ec;

    case CONT_STORAGE:
      for (i=0; i<count; i++) {
        TDCEvent ev(fBuffer[i]);
        trailing = ev.IsTrailing();
        value = (trailing) ? ev.GetTrailingTime() : ev.GetLeadingTime();
        channel = ev.GetChannelId();
        if (value != 0) {
          ec.push_back(ev);
          std::cout << std::dec
                    << "event " << i << "\t"
                    << "channel " << channel << "\t";
          switch(detm) {
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
            o.str(""); o << "Wrong detection mode: " << detm;
            throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
          }
          std::cout << std::endl;
        }
      }
      return ec;
      
    default:
      o.str(""); o << "Wrong acquisition mode: " << acqm;
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

  void
  TDCV1x90::WriteRegister(mod_reg addr, uint16_t* data) const
  {
    uint32_t address = fBaseAddr+addr;
    if (CAENVME_WriteCycle(fHandle, address, data, am, cvD16)!=cvSuccess) {
      std::ostringstream o; o << "Impossible to write register at 0x" << std::hex << addr;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  TDCV1x90::WriteRegister(mod_reg addr, uint32_t* data) const
  {
    uint32_t address = fBaseAddr+addr;
    if (CAENVME_WriteCycle(fHandle, address, data, am, cvD32)!=cvSuccess) {
      std::ostringstream o; o << "Impossible to write register at 0x" << std::hex << addr;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  TDCV1x90::ReadRegister(mod_reg addr, uint16_t* data) const
  {
    uint32_t address = fBaseAddr+addr;
    if (CAENVME_ReadCycle(fHandle, address, data, am, cvD16)!=cvSuccess) {
      std::ostringstream o; o << "Impossible to read register at 0x" << std::hex << addr;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  TDCV1x90::ReadRegister(mod_reg addr, uint32_t* data) const
  {
    uint32_t address = fBaseAddr+addr;
    if (CAENVME_ReadCycle(fHandle, address, data, am, cvD32)!=cvSuccess) {
      std::ostringstream o; o << "Impossible to read register at 0x" << std::hex << addr;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  bool
  TDCV1x90::WaitMicro(micro_handshake mode) const
  {
    uint16_t data;
    bool status = false;
    while (!status) {
      ReadRegister(MicroHandshake,&data);
      switch(mode){
        case WRITE_OK:
          status = static_cast<bool>(data&1);
          break;
        case READ_OK:
          status = static_cast<bool>((data&2)/2);
          break;
        default:
          return false;
      }
    }
    return status;
  }
}
