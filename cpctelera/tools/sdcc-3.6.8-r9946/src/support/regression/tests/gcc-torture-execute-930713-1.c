/*
   930713-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// Todo: Enable when sdcc supports returning struct!
#if 0
typedef struct
{
  char x;
} T;

T
f (s1)
     T s1;
{
  T s1a;
  s1a.x = 17;
  return s1a;
}
#endif

void
testTortureExecute (void)
{
#if 0
  T s1a, s1b;
  s1a.x = 13;
  s1b = f (s1a);
  if (s1a.x != 13 || s1b.x != 17)
    ASSERT (0);
  return;
#endif
}

