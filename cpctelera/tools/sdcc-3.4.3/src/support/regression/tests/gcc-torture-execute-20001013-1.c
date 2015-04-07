/*
   20001013-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct x {
	int a, b;
} z = { -4028, 4096 };

int foo(struct x *p, int y)
{
  if ((y & 0xff) != y || -p->b >= p->a)
    return 1;
  return 0;
}

void
testTortureExecute (void)
{
  if (foo (&z, 10))
    ASSERT (0);
  return;
}

