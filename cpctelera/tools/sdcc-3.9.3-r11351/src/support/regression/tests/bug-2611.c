/*
   bug-2611.c
 */

#include <testfwk.h>

void testBug(void)
{
#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Lack of memory
  volatile unsigned char c;
  volatile unsigned short x = 383;

  c = x * (0.275 * 2.0);
  ASSERT(c == 210);
  c = x * 0.55;
  ASSERT(c == 210);
  c = ((unsigned char) x) * 0.55;
  ASSERT(c == 69);
  c = ((unsigned char) x) * (0.275 * 2.0);
  ASSERT(c == 69);
  c = x / (25.0 / 10.0);
  ASSERT(c == 153);
  c = x / 2.50;
  ASSERT(c == 153);
#endif
}
