/*
divcmp-5.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* PR middle-end/26561 */

int always_one_1 (int a)
{
  if (a/100 >= -999999999)
    return 1;
  else
    return 0;
}

int always_one_2 (int a)
{
  if (a/100 < -999999999)
    return 0;
  else
    return 1;
}

void
testTortureExecute (void)
{
  if (always_one_1 (0) != 1)
    ASSERT (0);

  if (always_one_2 (0) != 1)
    ASSERT (0);

  return;
}

