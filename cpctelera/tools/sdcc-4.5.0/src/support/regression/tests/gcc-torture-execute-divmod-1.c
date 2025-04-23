/*
   divmod-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
div1 (signed char x)
{
  return x / -1;
}

int
div2 (signed short x)
{
  return x / -1;
}

int
div3 (signed char x, signed char y)
{
  return x / y;
}

int
div4 (signed short x, signed short y)
{
  return x / y;
}

int
mod1 (signed char x)
{
  return x % -1;
}

int
mod2 (signed short x)
{
  return x % -1;
}

int
mod3 (signed char x, signed char y)
{
  return x % y;
}

int
mod4 (signed short x, signed short y)
{
  return x % y;
}

#ifndef __SDCC_pdk14 // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
signed long
mod5 (signed long x, signed long y)
{
  return x % y;
}

unsigned long
mod6 (unsigned long x, unsigned long y)
{
  return x % y;
}
#endif
#endif

void
testTortureExecute (void)
{
  ASSERT(!(div1 (-(1 << 7)) != 1 << 7));
  ASSERT(!(div2 (-(1 << 15)) != 1 << 15));
  ASSERT(!(div3 (-(1 << 7), -1) != 1 << 7));
  ASSERT(!(div4 (-(1 << 15), -1) != 1 << 15));
  ASSERT(!(mod1 (-(1 << 7)) != 0));
  ASSERT(!(mod2 (-(1 << 15)) != 0));
  ASSERT(!(mod3 (-(1 << 7), -1) != 0));
  ASSERT(!(mod4 (-(1 << 15), -1) != 0));
#ifndef __SDCC_pdk14 // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
  ASSERT(!(mod5 (0x50000000, 2) != 0));
  ASSERT(!(mod6 (0x50000000, 2) != 0));
#endif
#endif

  return;
}
