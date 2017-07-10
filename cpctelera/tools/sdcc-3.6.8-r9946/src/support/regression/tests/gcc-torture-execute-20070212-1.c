/*
   20070212-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct f
{
  int i;
};

int g(int i, int c, struct f *ff, int *p)
{
  int *t;
  if (c)
   t = &i;
  else
   t = &ff->i;
  *p = 0;
  return *t;
}

void
testTortureExecute (void)
{
  struct f f;
  f.i = 1;
  if (g(5, 0, &f, &f.i) != 0)
    ASSERT (0);
  return;
}
