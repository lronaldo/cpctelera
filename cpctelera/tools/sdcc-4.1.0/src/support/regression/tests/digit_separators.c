/** 
  ISO C23 digit separators.
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c2x
#endif

void
testDigitSeparators(void)
{
#ifdef __SDCC
  ASSERT (1'120'377 == 1120377);
  ASSERT (0b0'00'11'00'01 == 0b000110001);
  ASSERT (0x01'ff'5a == 0x01ff5a);
  ASSERT (012'3'4 == 01234);
#endif
}

