/*
   20040311-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Copyright (C) 2004 Free Software Foundation.

   Check that constant folding and RTL simplification of -(x >> y) doesn't
   break anything and produces the expected results.

   Written by Roger Sayle, 11th March 2004.  */

#define INT_BITS  (sizeof(int)*8)

int ftest1(int x)
{
  return -(x >> (INT_BITS-1));
}

int ftest2(unsigned int x)
{
  return -((int)(x >> (INT_BITS-1)));
}

int ftest3(int x)
{
  int y;
  y = INT_BITS-1;
  return -(x >> y);
}

int ftest4(unsigned int x)
{
  int y;
  y = INT_BITS-1;
  return -((int)(x >> y));
}

void
testTortureExecute (void)
{
  if (ftest1(0) != 0)
    ASSERT (0);
  if (ftest1(1) != 0)
    ASSERT (0);
  if (ftest1(-1) != 1)
    ASSERT (0);

  if (ftest2(0) != 0)
    ASSERT (0);
  if (ftest2(1) != 0)
    ASSERT (0);
  if (ftest2((unsigned int)-1) != -1)
    ASSERT (0);

  if (ftest3(0) != 0)
    ASSERT (0);
  if (ftest3(1) != 0)
    ASSERT (0);
  if (ftest3(-1) != 1)
    ASSERT (0);

  if (ftest4(0) != 0)
    ASSERT (0);
  if (ftest4(1) != 0)
    ASSERT (0);
  if (ftest4((unsigned int)-1) != -1)
    ASSERT (0);

  return;
}

