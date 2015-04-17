#include "TDCConfiguration.h"

TDCConfiguration::TDCConfiguration() :
  fWord(0)
{}

void
TDCConfiguration::SetChannelOffset(int channel, short offset)
{
  //fWord |= 
}

short
TDCConfiguration::GetChannelOffset(int channel)
{
  if (channel>31 or channel<0) return -1;
  
  //return (fWord>>)
}

void
TDCConfiguration::Dump() const
{}
