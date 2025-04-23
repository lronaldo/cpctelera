/*
   shiftopt-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Copyright (C) 2002  Free Software Foundation

   Check that constant folding of shift operations is working.

   Roger Sayle, 10th October 2002.  */

extern void link_error (void);

void
utest (unsigned int x)
{
  if (x >> 0 != x)
    link_error ();

  if (x << 0 != x)
    link_error ();

  if (0 << x != 0)
    link_error ();

  if (0 >> x != 0)
    link_error ();

  if (-1 >> x != -1)
    link_error ();

  if (~0 >> x != ~0)
    link_error ();
}

void
stest (int x)
{
  if (x >> 0 != x)
    link_error ();

  if (x << 0 != x)
    link_error ();

  if (0 << x != 0)
    link_error ();

  if (0 >> x != 0)
    link_error ();
}

void
testTortureExecute (void)
{
  utest(9);
  utest(0);

  stest(9);
  stest(0);

  return;
}

void
link_error ()
{
  ASSERT (0);
}

