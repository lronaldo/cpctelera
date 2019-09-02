/*
   bug-2254.c a bug in compile-time evaluation of integer constant division.
 */

#include <testfwk.h>

void testBug(void)
{
#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Not enough RAM
  unsigned n = 5;
  volatile unsigned n2 = 5;
  volatile unsigned i = (unsigned) ( n / 2.5 );
  ASSERT(n2 / 2.5 == i);
#endif
}

