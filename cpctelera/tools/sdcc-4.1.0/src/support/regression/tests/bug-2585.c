/*
   bug-2585.c
   int cast issue
*/

#include <testfwk.h>

#include <limits.h>

void testBug (void)
{
  int i;

  i = SCHAR_MAX;

  ASSERT ((signed char)(i << (int)1) == -2);
}
