/*
   20011024-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Test whether store motion recognizes pure functions as potentially reading
   any memory.  */

#include <string.h>

char buf[50];

static void foo (void)
{
  if (memcpy (buf, "abc", 4) != buf) ASSERT (0);
  if (strcmp (buf, "abc")) ASSERT (0);
  memcpy (buf, "abcdefgh", strlen ("abcdefgh") + 1);
}

void
testTortureExecute (void)
{
  foo ();
  return;
}

