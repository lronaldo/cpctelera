/* bug-1805702.c
 */

#include <testfwk.h>

int foo;
extern int foo;

void
test(void)
{
#if !defined(__SDCC_pdk14) // Not enough RAM
  foo = 10;

  ASSERT(foo == 10);
#endif
}

#if !defined(__SDCC_pdk14) // Not enough RAM
/* compile time check for compiler defined functions (cdef) */

#ifndef __SDCC_mcs51
#define __nonbanked
#endif

float __fsmul (float, float) __nonbanked;

float __fsmul (float a1, float a2) __nonbanked {
  /* just for testing... */
  return (a1 + a2);
}
#endif

