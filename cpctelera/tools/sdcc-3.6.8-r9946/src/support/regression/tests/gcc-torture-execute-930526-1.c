/*
   930526-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>
#include <string.h>

#ifdef __SDCC
#pragma std_c99
#endif

inline
void f (int x)
{
  int *(p[3]);
  int m[3*4];
  int i;

  memset (m, 0x00, sizeof (m));

  for (i = 0; i < 3; i++)
    p[i] = m + x*i;

  p[0][2] = 0x5555;
  p[1][0] = 0x3333;
  p[2][1] = -23456;

  ASSERT (m[2] == 0x5555);
  ASSERT (m[4] == 0x3333);
  ASSERT (m[9] == -23456);
}

void
testTortureExecute (void)
{
  f (4);
  return;
}

extern inline void f (int x);

