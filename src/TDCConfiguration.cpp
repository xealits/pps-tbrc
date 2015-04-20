#include "TDCConfiguration.h"

TDCConfiguration::TDCConfiguration()
{
  for (uint8_t i=0; i<sizeof(fWord)/sizeof(fWord[0]); i++) {
    //fWord[i] = (1<<WORD_SIZE)-1;
    fWord[i] = 0;
  }
}

void
TDCConfiguration::SetBits(uint16_t lsb, uint16_t word, uint8_t size)
{
  //FIXME FIXME FIXME burp...
  // See http://www.ioccc.org/ for more information
  uint16_t lsb_rel = lsb % WORD_SIZE;
  uint8_t word_id = (lsb-lsb_rel)/WORD_SIZE;
  for (uint16_t i=0; i<size; i++) {
    fWord[word_id] &=~static_cast<word_t>(1<<(lsb_rel+i)); // first we clear the bit
    fWord[word_id] |= static_cast<word_t>(((word&(1<<i))>>i)<<(lsb_rel+i)); // then we set it
  }
}

uint16_t
TDCConfiguration::GetBits(uint16_t lsb, uint8_t size) const
{
  uint16_t lsb_rel = lsb % WORD_SIZE;
  uint8_t word_id = (lsb-lsb_rel)/WORD_SIZE;
  uint16_t mask = (1<<size)-1; // 2**size-1
  return static_cast<uint16_t>((fWord[word_id]>>lsb_rel) & mask);
}

void
TDCConfiguration::Dump(std::ostream& os) const
{
  os << "==========================================="
     << " TDC Configuration dump "
     << "===========================================" << std::endl << std::endl;
  for (unsigned int i=0; i<sizeof(fWord)/sizeof(fWord[0]); i++) {
    os << " Word " << std::setw(2) << i << ":  "
       << std::setw(3) << (i+1)*WORD_SIZE-1 << "-> |";
    for(int8_t j=WORD_SIZE-1; j>=0; j--) {
      uint16_t bit = j+i*WORD_SIZE;
      // bits values
      if (bit>=BITS_NUM) os << "x";
      else os << static_cast<bool>((fWord[i] & static_cast<word_t>(1<<j))>>j);
      // delimiters
      if (j%16==0 && j!=0) os << "| |";
      else if (j%8==0) os << "|";
      else if (j%4==0) os << " ";
    }
    os << " <-" << std::setw(3) << i*WORD_SIZE << std::endl;
  }
  os << std::endl
     << "======================================================="
     << "=======================================================" << std::endl;
}
