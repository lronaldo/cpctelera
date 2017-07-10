/*
   divcmp-4.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR middle-end/17894 */

int test1(int x)
{
  return x/-10 == 2;
}

int test2(int x)
{
  return x/-10 == 0;
}

int test3(int x)
{
  return x/-10 != 2;
}

int test4(int x)
{
  return x/-10 != 0;
}

int test5(int x)
{
  return x/-10 < 2;
}

int test6(int x)
{
  return x/-10 < 0;
}

int test7(int x)
{
  return x/-10  <= 2;
}

int test8(int x)
{
  return x/-10 <= 0;
}

int test9(int x)
{
  return x/-10 > 2;
}

int test10(int x)
{
  return x/-10 > 0;
}

int test11(int x)
{
  return x/-10 >= 2;
}

int test12(int x)
{
  return x/-10 >= 0;
}

void
testTortureExecute (void)
{
  if (test1(-30) != 0)
    ASSERT (0);
  if (test1(-29) != 1)
    ASSERT (0);
  if (test1(-20) != 1)
    ASSERT (0);
  if (test1(-19) != 0)
    ASSERT (0);

  if (test2(0) != 1)
    ASSERT (0);
  if (test2(9) != 1)
    ASSERT (0);
  if (test2(10) != 0)
    ASSERT (0);
  if (test2(-1) != 1)
    ASSERT (0);
  if (test2(-9) != 1)
    ASSERT (0);
  if (test2(-10) != 0)
    ASSERT (0);

  if (test3(-30) != 1)
    ASSERT (0);
  if (test3(-29) != 0)
    ASSERT (0);
  if (test3(-20) != 0)
    ASSERT (0);
  if (test3(-19) != 1)
    ASSERT (0);

  if (test4(0) != 0)
    ASSERT (0);
  if (test4(9) != 0)
    ASSERT (0);
  if (test4(10) != 1)
    ASSERT (0);
  if (test4(-1) != 0)
    ASSERT (0);
  if (test4(-9) != 0)
    ASSERT (0);
  if (test4(-10) != 1)
    ASSERT (0);

  if (test5(-30) != 0)
    ASSERT (0);
  if (test5(-29) != 0)
    ASSERT (0);
  if (test5(-20) != 0)
    ASSERT (0);
  if (test5(-19) != 1)
    ASSERT (0);

  if (test6(0) != 0)
    ASSERT (0);
  if (test6(9) != 0)
    ASSERT (0);
  if (test6(10) != 1)
    ASSERT (0);
  if (test6(-1) != 0)
    ASSERT (0);
  if (test6(-9) != 0)
    ASSERT (0);
  if (test6(-10) != 0)
    ASSERT (0);

  if (test7(-30) != 0)
    ASSERT (0);
  if (test7(-29) != 1)
    ASSERT (0);
  if (test7(-20) != 1)
    ASSERT (0);
  if (test7(-19) != 1)
    ASSERT (0);

  if (test8(0) != 1)
    ASSERT (0);
  if (test8(9) != 1)
    ASSERT (0);
  if (test8(10) != 1)
    ASSERT (0);
  if (test8(-1) != 1)
    ASSERT (0);
  if (test8(-9) != 1)
    ASSERT (0);
  if (test8(-10) != 0)
    ASSERT (0);

  if (test9(-30) != 1)
    ASSERT (0);
  if (test9(-29) != 0)
    ASSERT (0);
  if (test9(-20) != 0)
    ASSERT (0);
  if (test9(-19) != 0)
    ASSERT (0);

  if (test10(0) != 0)
    ASSERT (0);
  if (test10(9) != 0)
    ASSERT (0);
  if (test10(10) != 0)
    ASSERT (0);
  if (test10(-1) != 0)
    ASSERT (0);
  if (test10(-9) != 0)
    ASSERT (0);
  if (test10(-10) != 1)
    ASSERT (0);

  if (test11(-30) != 1)
    ASSERT (0);
  if (test11(-29) != 1)
    ASSERT (0);
  if (test11(-20) != 1)
    ASSERT (0);
  if (test11(-19) != 0)
    ASSERT (0);

  if (test12(0) != 1)
    ASSERT (0);
  if (test12(9) != 1)
    ASSERT (0);
  if (test12(10) != 0)
    ASSERT (0);
  if (test12(-1) != 1)
    ASSERT (0);
  if (test12(-9) != 1)
    ASSERT (0);
  if (test12(-10) != 1)
    ASSERT (0);

  return;
}

