/*
   pr68143_1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#define NULL 0

struct stuff
{
    int a;
    int b;
    int c;
    int d;
    int e;
    char *f;
    int g;
};

void
bar (struct stuff *x)
{
  if (x->g != 2)
    ASSERT (0);
}

void
testTortureExecute (void)
{
  struct stuff x = {0, 0, 0, 0, 0, NULL, 0};
  x.a = 100;
  x.d = 100;
  x.g = 2;
  /* Struct should now look like {100, 0, 0, 100, 0, 0, 0, 2}.  */
  bar (&x);
  return;
}
