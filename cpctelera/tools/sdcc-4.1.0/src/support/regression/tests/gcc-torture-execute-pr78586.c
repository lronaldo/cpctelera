/*
   pr78586.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdio.h>

/* PR tree-optimization/78586 */

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lackof memory
void
foo (unsigned long x)
{
  char a[30];
  unsigned long b = sprintf (a, "%lu", x);
  if (b != 4)
    ASSERT (0);
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lackof memory
  foo (1000);
  return;
#endif
}
