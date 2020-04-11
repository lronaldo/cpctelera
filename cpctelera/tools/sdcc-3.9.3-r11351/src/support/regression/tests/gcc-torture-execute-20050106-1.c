/*
   20050106-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/19283 */
#if !defined (__SDCC_pdk14) && !defined (__SDCC_pdk15) // Bug #2874
static inline unsigned short
foo (unsigned int *p)
{
  return *p;
}

unsigned int u;
#endif

void
testTortureExecute (void)
{
#if !defined (__SDCC_pdk14) && !defined (__SDCC_pdk15) // Bug #2874
  if ((foo (&u) & 0x8000) != 0)
    ASSERT (0);
  return;
#endif
}
