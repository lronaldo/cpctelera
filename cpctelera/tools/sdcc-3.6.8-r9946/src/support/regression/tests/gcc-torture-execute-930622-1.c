/*
   930622-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

int a = 1, b;

int g () { return 0; }
void h (int x) {}

int f ()
{
  if (g () == -1)
    return 0;
  a = g ();
  if (b >= 1)
    h (a);
  return 0;
}

void
testTortureExecute (void)
{
  f ();
  if (a != 0)
    ASSERT (0);
  return;
}

