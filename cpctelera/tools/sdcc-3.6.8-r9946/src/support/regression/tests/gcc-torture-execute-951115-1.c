/*
   951115-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int var = 0;

void g ()
{
  var = 1;
}

void f ()
{
  int f2 = 0;

  if (f2 == 0)
    ;

  g ();
}

void
testTortureExecute (void)
{
  f ();
  if (var != 1)
    ASSERT (0);
  return;
}

