/*
   20000717-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// Some ports do not support long long yet.
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_gbz80)
static void
compare (long long foo)
{
  if (foo < 4294967297LL)
    ASSERT (0);
}
#endif

void
testTortureExecute (void)
{
// Some ports do not support long long yet.
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_gbz80)
  compare (8589934591LL);
  return;
#endif
}

