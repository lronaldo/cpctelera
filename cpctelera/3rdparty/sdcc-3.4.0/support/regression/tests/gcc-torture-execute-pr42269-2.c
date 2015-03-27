/*
   pr42269-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
/* Make sure that language + abi extensions in passing S interoperate.  */

static long long
foo (unsigned short s)
{
  return (short) s;
}
#endif
unsigned short s = 0xFFFF;

void
testTortureExecute (void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  ASSERT (!(foo (s) + 1 != 0));
#endif
}

