/*
   compare-3.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Copyright (C) 2002 Free Software Foundation.

   Test for composite comparison always true/false optimization.

   Written by Roger Sayle, 7th June 2002.  */

extern void link_error0 ();
extern void link_error1 ();

void
test1 (int x, int y)
{
  if ((x==y) && (x!=y))
    link_error0();
}

void
test2 (int x, int y)
{
  if ((x<y) && (x>y))
    link_error0();
}

void
test3 (int x, int y)
{
  if ((x<y) && (y<x))
    link_error0();
}

void 
test4 (int x, int y)
{
  if ((x==y) || (x!=y))
    {
    }
  else
    link_error1 ();
}

void
test5 (int x, int y)
{
  if ((x>=y) || (x<y))
    {
    }
  else
    link_error1 ();
}

void
test6 (int x, int y)
{
  if ((x<=y) || (y<x))
    {
    }
  else
    link_error1 ();
}

void
all_tests (int x, int y)
{
  test1 (x, y);
  test2 (x, y);
  test3 (x, y);
  test4 (x, y);
  test5 (x, y);
  test6 (x, y);
}

void
testTortureExecute (void)
{
  all_tests (0, 0);
  all_tests (1, 2);
  all_tests (4, 3);

  return;
}

void link_error0() {ASSERT (0);}
void link_error1() {ASSERT (0);}


