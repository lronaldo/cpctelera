/*
   pr21331.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int bar (void) {  return -1;  }

unsigned long
foo ()
{ unsigned long retval;
  retval = bar ();
  if (retval == -1)  return 0;
  return 3;  }

void
testTortureExecute (void)
{
  if (foo () != 0)
    ASSERT (0);
  return;
}

