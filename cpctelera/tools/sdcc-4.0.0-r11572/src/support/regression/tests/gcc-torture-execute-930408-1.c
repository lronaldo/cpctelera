/*
   930408-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

typedef enum foo E;
enum foo { e0, e1 };

struct {
  E eval;
} s;

void p()
{
  ASSERT(0);
}

void f()
{
  switch (s.eval)
    {
    case e0:
      p();
    }
}

void
testTortureExecute (void)
{
  s.eval = e1;
  f();
  return;
}

