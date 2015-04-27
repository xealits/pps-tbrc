#include "TDCSetup.h"

void
TDCSetup::SetConstantValues()
{
  SetReadoutSingleCycleSpeed(RSC_40Mbits_s); // FIXME don't care...
  SetSerialDelay(0x0); // FIXME maybe affected by the realistic tests
  SetStrobeSelect(SS_LeadingEdge);
  SetReadoutSpeedSelect(RO_Fixed);
  SetTokenDelay(0x0); // FIXME maybe affected by the realistic tests
  
  SetEnableLocalTrailer(true); // FIXME not yet discussed...
  SetEnableLocalHeader(true); // FIXME not yet discussed...
  SetEnableGlobalTrailer(true); // FIXME not yet discussed...
  SetEnableGlobalHeader(true); // FIXME not yet discussed...
  
  SetKeepToken(true);
  SetMaster(true);
  SetEnableBytewise(true);
  
  SetBypassInputs(true);
  SetReadoutFIFOSize(256);
  
  SetEnableOverflowDetect(true);
  SetEnableRelative(false);
  SetEnableAutomaticReject(false);
  
  /*SetEventCountOffset(0); // FIXME needs confirmation
  SetTriggerCountOffset(0);*/
  
  SetEnableSetCountersOnBunchReset(false); // FIXME not yet discussed...
  SetEnableMasterResetCode(false);
  SetEnableMasterResetOnEventReset(false);
  SetEnableResetChannelBufferWhenSeparator(false); // FIXME not yet discussed...
  SetEnableSeparatorOnEventReset(false);
  SetEnableSeparatorOnBunchReset(false);
  SetEnableDirectEventReset(true);
  SetEnableDirectBunchReset(true);
  SetEnableDirectTrigger(true);
  
  SetLowPowerMode(true);
  SetDLLControl(0x1);
  
  //SetDeadTime(DT_5ns); // FIXME do we force the dead time value?
  //SetTestInvert(false);
  //SetTestMode(false);
  
  SetModeRCCompression(true);
  SetModeRC(true);
  SetDLLMode(DLL_320MHz);
  SetPLLControl(0x4, false, false, false);
  
  SetSerialClockDelay(false, 0x0);
  SetIOClockDelay(false, 0x0);
  SetCoreClockDelay(false, 0x0);
  SetDLLClockDelay(false, 0x0);
  SetSerialClockSource(Serial_pll_clock_80);
  SetIOClockSource(IO_clock_40);
  SetCoreClockSource(Core_pll_clock_80);
  SetDLLClockSource(DLL_pll_clock_320);
  
  SetRollOver(0xFFF);
  SetEnableTTLSerial(true);
  SetEnableTTLControl(true);
  SetEnableTTLReset(true);
  SetEnableTTLClock(false);
  SetEnableTTLHit(false);
}

void
TDCSetup::Dump(int verb, std::ostream& os) const
{
  os << "====================="
     << " TDC Setup register dump "
     << "====================" << std::endl;
     if (verb>1) DumpRegister(os);
  os << " Enabled errors:             ";
  for (unsigned int i=0; i<11; i++) {
    if (static_cast<bool>((GetEnableError()>>i)&0x1)) os << i << " ";
  }
  os << std::endl;
  os << " Edge resolution:            " << GetEdgeResolution() << std::endl
     << " Maximal event size:         " << GetMaxEventSize() << std::endl
     << " Reject events if FIFO full? " << GetRejectFIFOFull() << std::endl
     << " Channels offset/DLL adjustments:" << std::endl
     << "   +---------------------------------------------------------+" << std::endl;
  for (unsigned int i=0; i<TDC_NUM_CHANNELS/2; i++ ) {
    os << "   |  Ch.  " << std::setw(2) << i
       << ":   0x" << std::setfill('0')
       << std::setw(3) << std::hex << static_cast<int>(GetChannelOffset(i))
       << " / 0x"
       << std::setw(3) << static_cast<int>(GetDLLAdjustment(i)) << std::dec << std::setfill(' ')
       << "   |  Ch.  " << std::setw(2) << i+TDC_NUM_CHANNELS/2
       << ":   0x" << std::setfill('0')
       << std::setw(3) << std::hex << static_cast<int>(GetChannelOffset(i+TDC_NUM_CHANNELS/2))
       << " / 0x"
       << std::setw(3) << static_cast<int>(GetDLLAdjustment(i+TDC_NUM_CHANNELS/2)) << std::dec << std::setfill(' ')
       << " |" << std::endl;
  }
  os << "   +---------------------------------------------------------+" << std::endl
     << " Width resolution:           " << GetWidthResolution() << std::endl
     << " Dead time:                  " << GetDeadTime() << std::endl
     << " Leading/trailing mode:      " << GetLeadingMode() << " / " << GetTrailingMode() << std::endl
     << " Trigger matching mode:      " << GetTriggerMatchingMode() << std::endl
     << " Edges pairing:              " << GetEdgesPairing() << std::endl;
  os << "================================="
     << "=================================" << std::endl;
}
