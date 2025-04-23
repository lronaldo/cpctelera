/*
   931017-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int v;

void f (void);

void
testTortureExecute (void)
{
  f ();
  return;
}

int h1 (void)
{
  return 0;
}

int h2 (int *e)
{
  ASSERT(e == &v);
  return 0;
}

int g (char *c)
{
  int i;
  int b;

  do
    {
      i = h1 ();
      if (i == -1)
	return 0;
      else if (i == 1)
	h1 ();
    }
  while (i == 1);

  do
    b = h2 (&v);
  while (i == 5);

  if (i != 2)
    return b;
  *c = 'a';

  return 0;
}


void f (void)
{
  char c;
  g (&c);
}

