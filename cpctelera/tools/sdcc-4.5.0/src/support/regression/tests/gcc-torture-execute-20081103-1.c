/*
   20081103-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

struct S { char c; char arr[4]; float f; };

char A[4] = { '1', '2', '3', '4' };

void foo (struct S s)
{
  if (memcmp (s.arr, A, 4))
    ASSERT (0);
}

void
testTortureExecute (void)
{
  struct S s;
  memcpy (s.arr, A, 4);
  foo (s);
  return;
}

