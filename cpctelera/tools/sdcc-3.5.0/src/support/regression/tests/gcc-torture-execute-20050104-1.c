/*
   20050104-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

// Some ports do not support long long yet.
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_gbz80)

/* PR tree-optimization/19060 */

static
long long min ()
{
  return -LLONG_MAX - 1;
}

void
foo (long long j)
{
  if (j > 10 || j < min ())
    ASSERT (0);
}
#endif

void
testTortureExecute (void)
{
// Some ports do not support long long yet.
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_gbz80)
  foo (10);
  return;
#endif
}
