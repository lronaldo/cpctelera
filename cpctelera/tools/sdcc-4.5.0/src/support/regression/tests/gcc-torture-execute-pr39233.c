/*
   pr39233.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 127
#endif

void
foo (void *p)
{
  long l = (long) p;
  if (l < 0 || l > 6)
    ASSERT (0);
}

void
testTortureExecute (void)
{
  short i;
  for (i = 6; i >= 0; i--)
#if defined(__SDCC_pic14) // FIXME: maybe a bug in sdcc
    foo (i);
#else
    foo ((void *) (long) i);
#endif
  return;
}

