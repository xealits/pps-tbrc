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
    uint32_t model;
    uint16_t data[3];
    mod_reg addr[3] = {ROMBoard0, ROMBoard1, ROMBoard2};
    try {
      for (int i=0; i<3; i++) {
        ReadRegister(addr[i],&(data[i]));
      }
      model = (((data[2]&0xff) << 16)+((data[1]&0xff) << 8)+(data[0]&0xff));
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1)
      std::cout << "[VME] <TDC::GetModel> Debug: Model is " 
                << std::dec << model << std::endl;
    
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
      std::cout << "[VME] <TDC::GetOUI> Debug: OUI manufacturer number is " 
                << std::dec << oui << std::endl;
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
      std::cout << "[VME] <TDC::GetSerialNumber> Debug: Serial number is " 
                << std::dec << sn << std::endl;
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
      std::cout << "[VME] <TDC::GetFirmwareRev> Debug: Firmware revision is " 
                << std::dec << fr[1] << "." << fr[0] << std::endl;
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

  void TDCV1x90::SetLSBTraileadEdge(trailead_edge_lsb conf) {
    uint16_t word = conf;
    uint16_t value = TDCV1x90Opcodes::SET_TR_LEAD_LSB;
    try {
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&value);
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&word);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::cout << "[VME] <TDC::SetLSBTraileadEdge> Debug: ";
      switch(conf){
        case r800ps: std::cout << "800ps" << std::endl; break;
        case r200ps: std::cout << "200ps" << std::endl; break;
        case r100ps: std::cout << "100ps" << std::endl; break;
        case r25ps: std::cout << "25ps" << std::endl; break;
      }
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
      std::cout << "[VME] <TDC::SetGlobalOffset> Debug: " << std::endl;
      std::cout << "             coarse counter offset: " << word1 << std::endl;
      std::cout << "               fine counter offset: " << word2 << std::endl;
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
      std::cout << "[VME] <TDC::ReadGlobalOffset> Debug: " << std::endl;
      std::cout << "              coarse counter offset: " << data[0] << std::endl;
      std::cout << "                fine counter offset: " << data[1] << std::endl;
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
      std::cout << "[VME] <TDC::SetRCAdjust> Debug: TDC " << tdc
                << ", value " << value << std::endl;
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
      std::cout << "[VME] <TDC:ReadRCAdjust> Debug: value for TDC " << tdc << std::endl;
      double i;
      for(i=0;i<12;i++) {
        std::cout << "   bit " << std::setw(2) << i << ": ";
        char bit = (data&(uint16_t)(std::pow(2,i)));
        switch(bit) {
          case 0: std::cout << "contact open"; break;
          case 1: std::cout << "contact closed"; break;
        }
        std::cout << std::endl;
      }
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
      std::cout << "[VME] <TDC::SetDetection> Debug: ";
      switch(mode){
        case PAIR: std::cout << "pair mode" << std::endl; break;
        case OTRAILING: std::cout << "only trailing" << std::endl; break;
        case OLEADING: std::cout << "only leading" << std::endl; break;
        case TRAILEAD: std::cout << "trailing and leading" << std::endl; break;
      }
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
      std::cout << "[VME] <TDC:ReadDetection> Debug: ";
      switch(data){
        case PAIR: std::cout << "pair mode" << std::endl; break;
        case OTRAILING: std::cout << "only trailing" << std::endl; break;
        case OLEADING: std::cout << "only leading" << std::endl; break;
        case TRAILEAD: std::cout << "trailing and leading" << std::endl; break;
      }
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
      std::cout << "[VME] <TDC:ReadResolution> Debug: ";
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
        if (!(SetContinuousStorage()))
          std::cerr << "[VME] <TDC::SetContinuousStorage> ERROR: while entering the continuous storage mode" << std::endl;
        break;
      case TRIG_MATCH:
        if (!(SetTriggerMatching()))
          std::cerr << "[VME] <TDC::SetTriggerMatching> ERROR: while entering the trigger matching mode" << std::endl;
        break;
      default:
        std::cerr << "[VME] <TDC> ERROR: Wrong acquisition mode" << std::endl;
        break;
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
      std::cout << "[VME] <TDC::SetTriggerMatching> Debug: trigger matching mode"
                << std::endl;
    }
    return true;
  }

  bool
  TDCV1x90::IsTriggerMatching()
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
      std::cout << "[VME] <TDC::isTriggerMatching> Debug: value: "
          << data << " (";
      switch(data) {
        case 0: std::cout << "continuous storage"; break;
        case 1: std::cout << "trigger matching"; break;
        default: std::cout << "wrong answer!"; break;
      }
      std::cout << ")" << std::endl;
    }
    return (bool)data;
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
      std::cout << "[VME] <TDC::SetContinuousStorage> Debug: continuous storage mode" << std::endl;
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
      switch(value) {
        case true: buff+=std::pow(2,(double)reg); break;
        case false: buff-=std::pow(2,(double)reg); break;
      }
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
      switch(value) {
        case true: buff+=std::pow(2,(double)reg); break;
        case false: buff-=std::pow(2,(double)reg); break;
      }
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
    std::cout << "[VME] <TDC::WriteFIFOSize> Debug: WRITE_FIFO_SIZE: "
              << word << std::endl;
    uint16_t opcode = TDCV1x90Opcodes::SET_FIFO_SIZE;
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&opcode);
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&word);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::cout << "[VME] <TDC::WriteFIFOSize> Debug: WRITE_FIFO_SIZE: "
                << word << std::endl;
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
      std::cout << "[VME] <TDC::ReadFIFOSize> Debug: READ_FIFO_SIZE: "
                << std::dec << std::pow(2,data+1) << std::endl;
    }
  }

  void
  TDCV1x90::SetTDCEncapsulation(bool mode)
  {
    uint16_t opcode;
    switch(mode){
      case false:
        opcode = TDCV1x90Opcodes::DIS_HEAD_TRAILER;
        outBufTDCHeadTrail=false;
        break;
      case true:
        opcode = TDCV1x90Opcodes::EN_HEAD_TRAILER;
        outBufTDCHeadTrail=true;
        break;
    }
    try { 
      WaitMicro(WRITE_OK);
      WriteRegister(Micro,&opcode);
    } catch (Exception& e) { e.Dump(); }
    
    if (fVerb>1) {
      std::cout << "[VME] <TDC::SetTDCEncapsulation> Debug: Enabled? "
                << mode << std::endl;
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
      std::cout << "[VME] <TDC::GetTDCEncapsulation> Debug: READ_HEAD_TRAILER: "
                << enc << std::endl;
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
    switch(mode){
      case false:
        opcode = TDCV1x90Opcodes::DIS_ERROR_MARK;
        outBufTDCErr=false;
        break;
      case true:
        opcode = TDCV1x90Opcodes::EN_ERROR_MARK;
        outBufTDCErr=true;
        break;
    }
    WaitMicro(WRITE_OK);
    WriteRegister(Micro,&opcode);
    
    if (fVerb>1) {
      std::cout << "[VME] <TDC::SetTDCErrorMarks> Debug: Enabled? "
                << mode << std::endl;
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
      std::cout << "[VME] <TDC::SetBLTEventNumberRegister> Debug: value: "
                << value << std::endl;
    }
  }

  uint16_t
  TDCV1x90::GetBLTEventNumberRegister()
  {
    uint16_t value;
    try { ReadRegister(BLTEventNumber,&value); } catch (Exception& e) { e.Dump(); }
    if (fVerb>1) {
      std::cout << "[VME] <TDC::GetBLTEventNumberRegister> Debug: value: "
                << value << std::endl;
    }
    return value;
  }
    
  void
  TDCV1x90::SetETTT(bool mode)
  {
    SetCtlRegister(EXTENDED_TRIGGER_TIME_TAG_ENABLE,mode);
    outBufTDCTTT = mode;
    if (fVerb>1) {
      std::cout << "[VME] <TDC::SetETTT> Debug: Enabled? "
                << mode << std::endl;
    }
  }

  bool
  TDCV1x90::GetETTT()
  {
    return GetCtlRegister(EXTENDED_TRIGGER_TIME_TAG_ENABLE);
  }

  TDCEventCollection
  TDCV1x90::GetEvents()
  {
    TDCEventCollection ec;
    // Start Readout (check if BERR is set to 0)
    // Nw words are transmitted until the global TRAILER
    // Move file!
    
    memset(fBuffer, 0, sizeof(uint32_t));
    
    int count=0;
    int blts = 1024;
    bool finished;
    int i;

    CVErrorCodes ret;
    ret = CAENVME_BLTReadCycle(fHandle, fBaseAddr+0x0000, (char *)fBuffer, blts, am_blt, cvD32, &count);
    finished = ((ret==cvSuccess)||(ret==cvBusError)||(ret==cvCommError)); //FIXME investigate...
    if (finished && gEnd) {
      if (fVerb>1) {
        std::cout << "[VME] <TDC::GetEvents> Debug: Exit requested!" << std::endl;
      }
      exit(0);
    }
    if (acqm!=TRIG_MATCH) {
      std::ostringstream o; o << "Wrong acquisition mode: " << acqm;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
    
    for (i=0; i<count; i++) { // FIXME need to use the knowledge of the TDCEvent behaviour there...
      if ((fBuffer[i]>>27)==0x18) continue; // Filter out filler data
      ec.push_back(TDCEvent(fBuffer[i]));
    }
    return ec;
  }

  void
  TDCV1x90::abort()
  {
    if (fVerb>1) {
      std::cout << "[VME] <TDC::abort> Debug: received abort signal" << std::endl;
    }
    // Raise flag
    gEnd = true;
  }

  void
  TDCV1x90::WriteRegister(mod_reg addr, uint16_t* data)
  {
    uint32_t address = fBaseAddr+addr;
    if (CAENVME_WriteCycle(fHandle, address, data, am, cvD16)!=cvSuccess) {
      std::ostringstream o; o << "Impossible to write register at 0x" << std::hex << addr;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  TDCV1x90::WriteRegister(mod_reg addr, uint32_t* data)
  {
    uint32_t address = fBaseAddr+addr;
    if (CAENVME_WriteCycle(fHandle, address, data, am, cvD32)!=cvSuccess) {
      std::ostringstream o; o << "Impossible to write register at 0x" << std::hex << addr;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  TDCV1x90::ReadRegister(mod_reg addr, uint16_t* data)
  {
    uint32_t address = fBaseAddr+addr;
    if (CAENVME_ReadCycle(fHandle, address, data, am, cvD16)!=cvSuccess) {
      std::ostringstream o; o << "Impossible to read register at 0x" << std::hex << addr;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  void
  TDCV1x90::ReadRegister(mod_reg addr, uint32_t* data)
  {
    uint32_t address = fBaseAddr+addr;
    if (CAENVME_ReadCycle(fHandle, address, data, am, cvD32)!=cvSuccess) {
      std::ostringstream o; o << "Impossible to read register at 0x" << std::hex << addr;
      throw Exception(__PRETTY_FUNCTION__, o.str(), JustWarning);
    }
  }

  bool
  TDCV1x90::WaitMicro(micro_handshake mode)
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
