/*
   20020118-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* This tests an insn length of sign extension on h8300 port.  */

volatile signed char *q;
volatile signed int n;

void
foo (void)
{
  signed char *p;

  for (;;)
    {
      p = (signed char *) q; n = p[2];
      p = (signed char *) q; n = p[2];
      p = (signed char *) q; n = p[2];
      p = (signed char *) q; n = p[2];
      p = (signed char *) q; n = p[2];
      p = (signed char *) q; n = p[2];
      p = (signed char *) q; n = p[2];
      p = (signed char *) q; n = p[2];
      p = (signed char *) q; n = p[2];
      p = (signed char *) q; n = p[2];
      p = (signed char *) q; n = p[2];
    }
}

void
testTortureExecute (void)
{
}
