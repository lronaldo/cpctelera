/*
   bug-2539.c
*/

#include <testfwk.h>

#include <stdint.h>

struct sBITS {
  unsigned  b0 : 1;
  unsigned  b1 : 1;
  unsigned  b2 : 1;
  unsigned  b3 : 1;
  unsigned  b4 : 1;
  unsigned  b5 : 1;
  unsigned  b6 : 1;
  unsigned  b7 : 1;
};
struct sBITS vb;

void SetIf(uint8_t x)
{
  vb.b3 = (x != 0); // only bit 0 of 'x' testet
}   

void testBug(void)
{
  SetIf(0);

  ASSERT(!vb.b3);

  SetIf(24);

  ASSERT(vb.b3);
}

