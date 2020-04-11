/*
bitfld-5.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* See http://gcc.gnu.org/ml/gcc/2009-06/msg00072.html.  */

#if 0 // Enable when SDCC supports bit-fields wider than 16 bits

struct s
{
  unsigned long long a:2;
  unsigned long long b:40;
  unsigned long long c:22;
};

void
g (unsigned long long a, unsigned long long b)
{
#ifdef __SDCC
  __asm;
  __endasm;
#endif
  if (a != b)
    ASSERT (0);
}

void
f (struct s s, unsigned long long b)
{
#ifdef __SDCC
  __asm;
  __endasm;
#endif
  g (((unsigned long long) (s.b-8)) + 8, b);
}
#endif

void
testTortureExecute (void)
{
#if 0 // Enable when SDCC supports bit-fields wider than 16 bits
  struct s s = {1, 10, 3};
  struct s t = {1, 2, 3};
  f (s, 10);
  f (t, 0x10000000002);
#endif
  return;
}
