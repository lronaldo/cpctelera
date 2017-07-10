/*
   20000731-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

double
foo (void)
{
  return 0.0;
}

void
do_sibcall (void)
{
  (void) foo ();
}

void
testTortureExecute (void)
{
   double x;

   for (x = 0; x < 20; x++)
      do_sibcall ();
   if (!(x >= 10))
      ASSERT (0);
   return;
}

