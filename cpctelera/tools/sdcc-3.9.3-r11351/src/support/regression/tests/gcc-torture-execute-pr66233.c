/*
   pr66233.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

/* PR tree-optimization/66233 */

unsigned int v[8];

#ifndef __SDCC_pdk14 // Lack of memory
void
foo (void)
{
  int i;
  for (i = 0; i < 8; i++)
    v[i] = (float) i;
}
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  unsigned int i;
  foo ();
  for (i = 0; i < 8; i++)
    if (v[i] != i)
      ASSERT (0);
  return;
#endif
}
