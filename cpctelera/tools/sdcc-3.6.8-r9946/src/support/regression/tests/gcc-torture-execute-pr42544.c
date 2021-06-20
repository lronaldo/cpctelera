/*
   pr42544.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR c/42544 */

extern void abort (void);

void
testTortureExecute (void)
{
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
// TODO: Enable once sdcc really support long long literals.
  signed short s = -1;
  if (sizeof (long long) == sizeof (unsigned int))
    return;
#ifndef PORT_HOST // Fails on Solaris for host - probably an old gcc version.
  if ((unsigned int) s >= 0x100000000ULL)
    ASSERT (0);
#endif
  return;
#endif
}

