/** getbyte_param.c - test for bug #3269
*/
#include <testfwk.h>
#include <stdlib.h>

void
fCheckPlus (unsigned char c)
{
  ASSERT (c == 0x15);
}
void
fCheckMinus (unsigned char c)
{
  ASSERT (c == 0x13);
}
void
getByteParam (unsigned int i)
{
  unsigned char c;

  c = i >> 8;
  c++;
  fCheckPlus(c);
  c = i >> 8;
  c--;
  fCheckMinus(c);
}

void
testGetByte (void)
{
  getByteParam(0x147A);
}