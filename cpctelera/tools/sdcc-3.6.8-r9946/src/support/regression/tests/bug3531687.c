/*
  bug3531687.c

  The bug resulted in wrong comparisons of char to bool, casting the char operand to bool.
*/

#include <testfwk.h>
#include <stdbool.h>

#pragma std_c99

int f(char a, char flag)
{
  if (a == (flag ? 1 : 0))
    return 0;
  return 1;
}

bool g(char a, bool b)
{
  return (a > b);
}

void
testBug(void)
{
  ASSERT(f(2, 1));
  ASSERT(g(2, 1));
}
