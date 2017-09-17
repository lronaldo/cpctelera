/*
   930929-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int sub1 (int i)
{
  return i - (5 - i);
}

int sub2 (int i)
{
  return i + (5 + i);
}

int sub3 (int i)
{
  return i - (5 + i);
}

int sub4 (int i)
{
  return i + (5 - i);
}

void
testTortureExecute (void)
{
  if (sub1 (20) != 35)
    ASSERT (0);
  if (sub2 (20) != 45)
    ASSERT (0);
  if (sub3 (20) != -5)
    ASSERT (0);
  if (sub4 (20) != 5)
    ASSERT (0);
  return;
}

