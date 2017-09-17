/*
   int-compare.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c89
#endif

#include <limits.h>

int gt (int a, int b)
{
  return a > b;
}

int ge (int a, int b)
{
  return a >= b;
}

int lt (int a, int b)
{
  return a < b;
}

int le (int a, int b)
{
  return a <= b;
}

void
true (int c)
{
  if (!c)
    ASSERT (0);
}

void
false (int c)
{
  if (c)
    ASSERT (0);
}

void f (void)
{
  true (gt (2, 1));
  false (gt (1, 2));

  true (gt (INT_MAX, 0));
  false (gt (0, INT_MAX));
  true (gt (INT_MAX, 1));
  false (gt (1, INT_MAX));

  false (gt (INT_MIN, 0));
  true (gt (0, INT_MIN));
  false (gt (INT_MIN, 1));
  true (gt (1, INT_MIN));

  true (gt (INT_MAX, INT_MIN));
  false (gt (INT_MIN, INT_MAX));

  true (ge (2, 1));
  false (ge (1, 2));

  true (ge (INT_MAX, 0));
  false (ge (0, INT_MAX));
  true (ge (INT_MAX, 1));
  false (ge (1, INT_MAX));

  false (ge (INT_MIN, 0));
  true (ge (0, INT_MIN));
  false (ge (INT_MIN, 1));
  true (ge (1, INT_MIN));

  true (ge (INT_MAX, INT_MIN));
  false (ge (INT_MIN, INT_MAX));

  false (lt (2, 1));
  true (lt (1, 2));

  false (lt (INT_MAX, 0));
  true (lt (0, INT_MAX));
  false (lt (INT_MAX, 1));
  true (lt (1, INT_MAX));

  true (lt (INT_MIN, 0));
  false (lt (0, INT_MIN));
  true (lt (INT_MIN, 1));
  false (lt (1, INT_MIN));

  false (lt (INT_MAX, INT_MIN));
  true (lt (INT_MIN, INT_MAX));

  false (le (2, 1));
  true (le (1, 2));

  false (le (INT_MAX, 0));
  true (le (0, INT_MAX));
  false (le (INT_MAX, 1));
  true (le (1, INT_MAX));

  true (le (INT_MIN, 0));
  false (le (0, INT_MIN));
  true (le (INT_MIN, 1));
  false (le (1, INT_MIN));

  false (le (INT_MAX, INT_MIN));
  true (le (INT_MIN, INT_MAX));
}

void
testTortureExecute (void)
{
  f ();
  return;
}

