/* 
  Bug #3482146.
*/

#include <testfwk.h>

void
testBug(void)
{
#if !defined(__SDCC_mcs51)
  unsigned char buff[176];
  unsigned char i;
  for(i = 0; i < 22*8; ++i)
    buff[i] = 0xff;
  buff[0] = buff[1] = 0x42;
  i = buff[0] - buff[1]; // The bug doesn't occur if you just use "i=0;"

  buff[88 + i] = buff[16 + i];

  buff[8 + i] = buff[i] >> 1;
  buff[16 + i] = buff[8+i] >> 1;	// This operation will not write into the right memory address

  ASSERT(buff[16] == 0x10);
#endif
}

