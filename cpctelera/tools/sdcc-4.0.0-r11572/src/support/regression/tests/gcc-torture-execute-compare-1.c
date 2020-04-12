/*
   compare-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Copyright (C) 2002 Free Software Foundation.

   Test for correctness of composite comparisons.

   Written by Roger Sayle, 3rd June 2002.  */

int ieq (int x, int y, int ok)
{
  if ((x<=y) && (x>=y))
    {
      if (!ok) ASSERT (0);
    }
  else
    if (ok) ASSERT (0);

  if ((x<=y) && (x==y))
    {
      if (!ok) ASSERT (0);
    }
  else
    if (ok) ASSERT (0);

  if ((x<=y) && (y<=x))
    {
      if (!ok) ASSERT (0);
    }
  else
    if (ok) ASSERT (0);

  if ((y==x) && (x<=y))
    {
      if (!ok) ASSERT (0);
    }
  else
    if (ok) ASSERT (0);
}

int ine (int x, int y, int ok)
{
  if ((x<y) || (x>y))
    {
      if (!ok) ASSERT (0);
    }
  else
    if (ok) ASSERT (0);
}

int ilt (int x, int y, int ok)
{
  if ((x<y) && (x!=y))
    {
      if (!ok) ASSERT (0);
    }
  else
    if (ok) ASSERT (0);
}

int ile (int x, int y, int ok)
{
  if ((x<y) || (x==y))
    {
      if (!ok) ASSERT (0);
    }
  else
    if (ok) ASSERT (0);
}

int igt (int x, int y, int ok)
{
  if ((x>y) && (x!=y))
    {
      if (!ok) ASSERT (0);
    }
  else
    if (ok) ASSERT (0);
}

int ige (int x, int y, int ok)
{
  if ((x>y) || (x==y))
    {
      if (!ok) ASSERT (0);
    }
  else
    if (ok) ASSERT (0);
}

void
testTortureExecute (void)
{
  ieq (1, 4, 0);
  ieq (3, 3, 1);
  ieq (5, 2, 0);

  ine (1, 4, 1);
  ine (3, 3, 0);
  ine (5, 2, 1);

  ilt (1, 4, 1);
  ilt (3, 3, 0);
  ilt (5, 2, 0);

  ile (1, 4, 1);
  ile (3, 3, 1);
  ile (5, 2, 0);

  igt (1, 4, 0);
  igt (3, 3, 0);
  igt (5, 2, 1);

  ige (1, 4, 0);
  ige (3, 3, 1);
  ige (5, 2, 1);

  return;
}

