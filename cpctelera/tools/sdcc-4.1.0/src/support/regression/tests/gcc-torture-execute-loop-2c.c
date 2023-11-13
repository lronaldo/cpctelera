/*
   loop-2c.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int a[2];

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Bug #2874
static
inline void f (int b, int o)
{
  unsigned int i;
  int *p;
  for (p = &a[b], i = b; --i < ~0; )
    *--p = i * 3 + o;
}

void g(int b)
{
  f (b, (int)a);
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Bug #2874
  a[0] = a[1] = 0;
  g (2);
  if (a[0] != (int)a || a[1] != (int)a + 3)
    ASSERT (0);
  return;
#endif
}
