/*
   enum-3.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* The composite type of int and an enum compatible with int might be
   either of the two types, but it isn't an unsigned type.  */
/* Origin: Joseph Myers <jsm@polyomino.org.uk> */

#include <limits.h>

#include <stdio.h>

enum e { a = INT_MIN };

int *p;
enum e *q;

void
testTortureExecute (void)
{
  enum e x = a;
  q = &x;
  if (*(1 ? q : p) > 0)
    ASSERT (0);
  return;
}

