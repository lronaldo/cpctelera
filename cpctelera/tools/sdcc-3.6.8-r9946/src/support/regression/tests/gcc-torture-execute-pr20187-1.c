/*
   pr207187-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// Some ports do not support long long yet.
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
int a = 0x101;
int b = 0x100;

int
foo (void)
{
  return (((unsigned char) (unsigned long long) ((a ? a : 1) & (a * b)))
	  ? 0 : 1);
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  ASSERT (!(1 - foo ()));
#endif
}

