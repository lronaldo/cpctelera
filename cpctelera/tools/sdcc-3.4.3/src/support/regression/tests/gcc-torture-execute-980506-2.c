/*
   980506-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

static void *self(void *p){ return p; }

int
f()
{
  struct { int i; } s, *sp;
  int *ip = &s.i;

  s.i = 1;
  sp = self(&s);
  
  *ip = 0;
  return sp->i+1;
}

void
testTortureExecute (void)
{
  if (f () != 1)
    ASSERT (0);
  else
    return;
}

