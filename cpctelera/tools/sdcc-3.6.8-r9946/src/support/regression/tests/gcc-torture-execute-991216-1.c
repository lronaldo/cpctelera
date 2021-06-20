/*
   991216-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 184
#endif

// Some ports do not support long long yet.
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
// Not enough memory
#if !(defined (__SDCC_mcs51) && defined (__SDCC_MODEL_SMALL))

#define VALUE 0x123456789abcdefLL
#define AFTER 0x55

void
test1 (int a, long long value, int after)
{
  if (a != 1
      || value != VALUE
      || after != AFTER)
    ASSERT (0);
}

void
test2 (int a, int b, long long value, int after)
{
  if (a != 1
      || b != 2
      || value != VALUE
      || after != AFTER)
    ASSERT (0);
}

void
test3 (int a, int b, int c, long long value, int after)
{
  if (a != 1
      || b != 2
      || c != 3
      || value != VALUE
      || after != AFTER)
    ASSERT (0);
}

void
test4 (int a, int b, int c, int d, long long value, int after)
{
  if (a != 1
      || b != 2
      || c != 3
      || d != 4
      || value != VALUE
      || after != AFTER)
    ASSERT (0);
}

void
test5 (int a, int b, int c, int d, int e, long long value, int after)
{
  if (a != 1
      || b != 2
      || c != 3
      || d != 4
      || e != 5
      || value != VALUE
      || after != AFTER)
    ASSERT (0);
}

void
test6 (int a, int b, int c, int d, int e, int f, long long value, int after)
{
  if (a != 1
      || b != 2
      || c != 3
      || d != 4
      || e != 5
      || f != 6
      || value != VALUE
      || after != AFTER)
    ASSERT (0);
}

void
test7 (int a, int b, int c, int d, int e, int f, int g, long long value, int after)
{
  if (a != 1
      || b != 2
      || c != 3
      || d != 4
      || e != 5
      || f != 6
      || g != 7
      || value != VALUE
      || after != AFTER)
    ASSERT (0);
}

void
test8 (int a, int b, int c, int d, int e, int f, int g, int h, long long value, int after)
{
  if (a != 1
      || b != 2
      || c != 3
      || d != 4
      || e != 5
      || f != 6
      || g != 7
      || h != 8
      || value != VALUE
      || after != AFTER)
    ASSERT (0);
}
#endif
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
#if !(defined (__SDCC_mcs51) && defined (__SDCC_MODEL_SMALL))
  test1 (1, VALUE, AFTER);
  test2 (1, 2, VALUE, AFTER);
  test3 (1, 2, 3, VALUE, AFTER);
  test4 (1, 2, 3, 4, VALUE, AFTER);
  test5 (1, 2, 3, 4, 5, VALUE, AFTER);
  test6 (1, 2, 3, 4, 5, 6, VALUE, AFTER);
  test7 (1, 2, 3, 4, 5, 6, 7, VALUE, AFTER);
  test8 (1, 2, 3, 4, 5, 6, 7, 8, VALUE, AFTER);
  return;
#endif
#endif
}

