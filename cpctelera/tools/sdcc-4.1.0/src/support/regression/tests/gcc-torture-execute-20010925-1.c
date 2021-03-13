/*
   20010925-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

int foo (void *, void *, unsigned int c);

int src[10];
int dst[10];

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
   if (foo (dst, src, 10) != 0)
     ASSERT (0);
   return;
#endif
}

#ifndef __SDCC_pdk14 // Lack of memory
int foo (void *a, void *b, unsigned int c)
{
  if (c == 0)
    return 1;

  memcpy (a, b, c);
  return 0;
}
#endif
