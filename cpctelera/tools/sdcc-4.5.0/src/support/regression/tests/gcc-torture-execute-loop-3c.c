/*
   loop-4c.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

#if (!defined(__SDCC_mcs51) || defined(__SDCC_MODEL_LARGE) || defined(__SDCC_MODEL_HUGE)) && !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
void * a[255];

void f (int m)
{
  int i;
  int sh = 0x100;
  i = m;
  do
    {
      a[sh >>= 1] = ((unsigned)i << 3)  + (char*)a;
      i += 4;
    }
  while (i < INT_MAX/2 + 1 + 4 * 4);
}
#endif

void
testTortureExecute (void)
{
#if (!defined(__SDCC_mcs51) || defined(__SDCC_MODEL_LARGE) || defined(__SDCC_MODEL_HUGE)) && !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  a[0x10] = 0;
  a[0x08] = 0;
  f (INT_MAX/2 + INT_MAX/4 + 2);
  if (a[0x10] || a[0x08])
    ASSERT (0);
  a[0x10] = 0;
  a[0x08] = 0;
  f (INT_MAX/2 + 1);
  if (! a[0x10] || a[0x08])
    ASSERT (0);
#endif
  return;
}

