/*
   940122-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

char *a = 0;
char *b = 0;

void g (int x)
{
  if ((!!a) != (!!b))
    ASSERT (0);
}

void f (int x)
{
  g (x * x);
}

void
testTortureExecute (void)
{
  f (100);
  return;
}

