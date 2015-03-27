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
   if (foo (dst, src, 10) != 0)
     ASSERT (0);
   return;
}

int foo (void *a, void *b, unsigned int c)
{
  if (c == 0)
    return 1;

  memcpy (a, b, c);
  return 0;
}

