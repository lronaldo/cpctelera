/*
   20041114-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Verify that
   
   var <= 0 || ((long unsigned) (unsigned) (var - 1) < MAX_UNSIGNED_INT)

   gets folded to 1.  */

#include <limits.h>

void link_failure (void);

volatile int v;

void 
foo (int var)
{
  if (!(var <= 0
        || ((long unsigned) (unsigned) (var - 1) < UINT_MAX)))
    link_failure ();
}

void
testTortureExecute (void)
{
  foo (v);
  return;
}

void
link_failure (void)
{
  ASSERT (0);
}

