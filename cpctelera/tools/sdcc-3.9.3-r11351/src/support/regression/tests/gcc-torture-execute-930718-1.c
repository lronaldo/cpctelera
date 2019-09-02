/*
   930718-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

typedef struct rtx_def
{
  int f1 :1;
  int f2 :1;
} *rtx;

static rtx
f (register rtx orig)
{
  if (orig->f1 || orig->f2)
    return orig;
  orig->f2 = 1;
  return orig;
}

void
f2 ()
{
  ASSERT (0);
}

void
testTortureExecute (void)
{
  struct rtx_def foo;
  rtx bar;

  foo.f1 = 1;
  foo.f2 = 0;
  bar = f (&foo);
  if (bar != &foo || bar->f2 != 0)
    ASSERT (0);
  return;
}

