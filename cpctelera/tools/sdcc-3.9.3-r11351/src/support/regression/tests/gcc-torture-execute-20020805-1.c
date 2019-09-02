/*
   20020805-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* This testcase was miscompiled on IA-32, because fold-const
   assumed associate_trees is always done on PLUS_EXPR.  */

void check (unsigned int m)
{
  if (m != (unsigned int) -1)
    ASSERT (0);
}

unsigned int n = 1;

void testTortureExecute (void)
{
  unsigned int m;
  m = (1 | (2 - n)) | (-n);
  check (m);
  return;
}
