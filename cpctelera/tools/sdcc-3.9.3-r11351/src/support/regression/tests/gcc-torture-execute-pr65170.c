/*
   pr65170.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/65170 */

typedef unsigned long long int V;
typedef unsigned int H;

#ifndef __SDCC_pdk14 // Lack of memory
void
foo (V b, V c)
{
  V a;
  b &= (H) -1;
  c &= (H) -1;
  a = b * c;
  if (a != 1)
    ASSERT (0);
}
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  foo (1, 1);
  return;
#endif
}

