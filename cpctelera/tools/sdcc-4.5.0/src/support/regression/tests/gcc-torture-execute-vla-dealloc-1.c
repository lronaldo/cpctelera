/*
vla-dealloc-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* VLAs should be deallocated on a jump to before their definition,
   including a jump to a label in an inner scope.  PR 19771.  */
/* { dg-require-effective-target alloca } */

#define LIMIT 10000

void *volatile p;

void
testTortureExecute (void)
{
#if 0 // TODO: Enable when SDCC supports VLAs!
  int n = 0;
  if (0)
    {
    lab:;
    }
  int x[n % 1000 + 1];
  x[0] = 1;
  x[n % 1000] = 2;
  p = x;
  n++;
  if (n < LIMIT)
    goto lab;
#endif
  return;
}
