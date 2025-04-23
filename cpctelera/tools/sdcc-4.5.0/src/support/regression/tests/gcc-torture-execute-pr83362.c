/*
pr83362.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;

#if !defined(__SDCC_pdk14) // Lack of memory
u32 a, b, d, e;
u8 c;

static u32
foo (u32 p)
{
  do
    {
      e /= 0xfff;
      if (p > c)
	d = 0;
      e -= 3;
      e *= b <= a;
    }
  while (e >= 88030);
  return e;
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) // Lack of memory
  u32 x = foo (1164);
  if (x != 0xfd)
    ASSERT (0);
  return;
#endif
}


