/*
   20000722-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable whens dcc supports this!
#if 0
struct s { char *p; int t; };

extern void bar (void);
extern void foo (struct s *);
#endif

void
testTortureExecute (void)
{
#if 0
  bar ();
  bar ();
  return;
#endif
}

#if 0
void 
bar (void)
{
  foo (& (struct s) { "hi", 1 });
}
#endif

#if 0
void foo (struct s *p)
{
  if (p->t != 1)
    ASSERT (0);
  p->t = 2;
}
#endif
