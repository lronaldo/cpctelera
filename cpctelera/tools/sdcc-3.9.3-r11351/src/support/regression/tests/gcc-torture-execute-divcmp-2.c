/*
   divcmp-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int test1(int x)
{
  return x/10 == 2;
}

int test2(int x)
{
  return x/10 == 0;
}

int test3(int x)
{
  return x/10 == -2;
}

int test4(int x)
{
  return x/-10 == 2;
}

int test5(int x)
{
  return x/-10 == 0;
}

int test6(int x)
{
  return x/-10 == -2;
}

void
testTortureExecute (void)
{
  if (test1(19) != 0)
    ASSERT (0);
  if (test1(20) != 1)
    ASSERT (0);
  if (test1(29) != 1)
    ASSERT (0);
  if (test1(30) != 0)
    ASSERT (0);

  if (test2(-10) != 0)
    ASSERT (0);
  if (test2(-9) != 1)
    ASSERT (0);
  if (test2(9) != 1)
    ASSERT (0);
  if (test2(10) != 0)
    ASSERT (0);

  if (test3(-30) != 0)
    ASSERT (0);
  if (test3(-29) != 1)
    ASSERT (0);
  if (test3(-20) != 1)
    ASSERT (0);
  if (test3(-19) != 0)
    ASSERT (0);

  if (test4(-30) != 0)
    ASSERT (0);
  if (test4(-29) != 1)
    ASSERT (0);
  if (test4(-20) != 1)
    ASSERT (0);
  if (test4(-19) != 0)
    ASSERT (0);

  if (test5(-10) != 0)
    ASSERT (0);
  if (test5(-9) != 1)
    ASSERT (0);
  if (test5(9) != 1)
    ASSERT (0);
  if (test5(10) != 0)
    ASSERT (0);

  if (test6(19) != 0)
    ASSERT (0);
  if (test6(20) != 1)
    ASSERT (0);
  if (test6(29) != 1)
    ASSERT (0);
  if (test6(30) != 0)
    ASSERT (0);

  return;
}

