/*
   pr68376-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

/* PR rtl-optimization/68376 */

int
f1 (int x)
{
  return x < 0 ? ~x : x;
}

int
f2 (int x)
{
  return x < 0 ? x : ~x;
}

int
f3 (int x)
{
  return x <= 0 ? ~x : x;
}

int
f4 (int x)
{
  return x <= 0 ? x : ~x;
}

int
f5 (int x)
{
  return x >= 0 ? ~x : x;
}

int
f6 (int x)
{
  return x >= 0 ? x : ~x;
}

int
f7 (int x)
{
  return x > 0 ? ~x : x;
}

int
f8 (int x)
{
  return x > 0 ? x : ~x;
}

void
testTortureExecute (void)
{
  if (f1 (5) != 5 || f1 (-5) != 4 || f1 (0) != 0)
    ASSERT (0);
  if (f2 (5) != -6 || f2 (-5) != -5 || f2 (0) != -1)
    ASSERT (0);
  if (f3 (5) != 5 || f3 (-5) != 4 || f3 (0) != -1)
    ASSERT (0);
  if (f4 (5) != -6 || f4 (-5) != -5 || f4 (0) != 0)
    ASSERT (0);
  if (f5 (5) != -6 || f5 (-5) != -5 || f5 (0) != -1)
    ASSERT (0);
  if (f6 (5) != 5 || f6 (-5) != 4 || f6 (0) != 0)
    ASSERT (0);
  if (f7 (5) != -6 || f7 (-5) != -5 || f7 (0) != 0)
    ASSERT (0);
  if (f8 (5) != 5 || f8 (-5) != 4 || f8 (0) != -1)
    ASSERT (0);
  return;
}
