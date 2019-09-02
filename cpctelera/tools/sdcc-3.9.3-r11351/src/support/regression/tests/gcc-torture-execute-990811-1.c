/*
   990811-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct s {long a; int b;};

int foo(int x, void *y)
{
  switch(x) {
    case 0: return ((struct s*)y)->a;
    case 1: return *(signed char*)y;
    case 2: return *(short*)y;
  }
  ASSERT(0);
}

void
testTortureExecute (void)
{
  struct s s;
  short sh[10];
  signed char c[10];
  int i;

  s.a = 1;
  s.b = 2;
  for (i = 0; i < 10; i++) {
    sh[i] = i;
    c[i] = i;
  }

  if (foo(0, &s) != 1) ASSERT(0);
  if (foo(1, c+3) != 3) ASSERT(0);
  if (foo(2, sh+3) != 3) ASSERT(0);
  return;
}

