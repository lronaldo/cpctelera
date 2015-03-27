/*
   20030120-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* On H8/300 port, NOTICE_UPDATE_CC had a bug that causes the final
   pass to remove test insns that should be kept.  */

unsigned short
t1 (unsigned short w)
{
  if ((w & 0xff00) == 0)
    {
      if (w == 0)
	w = 2;
    }
  return w;
}

unsigned long
t2 (unsigned long w)
{
  if ((w & 0xffff0000) == 0)
    {
      if (w == 0)
	w = 2;
    }
  return w;
}

int
t3 (unsigned short a)
{
  if (a & 1)
    return 1;
  else if (a)
    return 1;
  else
    return 0;
}

void
testTortureExecute (void)
{
  if (t1 (1) != 1)
    ASSERT (0);

  if (t2 (1) != 1)
    ASSERT (0);

  if (t3 (2) != 1)
    ASSERT (0);

  return;
}

