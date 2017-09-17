/*
   20071219-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

/* PR c++/34459 */

struct S
{
  char s[25];
};

struct S *p;

void
foo (struct S *x, int set)
{
  int i;
  for (i = 0; i < sizeof (x->s); ++i)
    if (x->s[i] != 0)
      ASSERT (0);
    else if (set)
      x->s[i] = set;
  p = x;
}

#if 0 // TODO: Enable when intermingeld declarations in code are supported!
void
ftest1 (void)
{
  struct S a;
  memset (&a.s, '\0', sizeof (a.s));
  foo (&a, 0);
  struct S b = a;
  foo (&b, 1);
  b = a;
  b = b;
  foo (&b, 0);
}

void
ftest2 (void)
{
  struct S a;
  memset (&a.s, '\0', sizeof (a.s));
  foo (&a, 0);
  struct S b = a;
  foo (&b, 1);
  b = a;
  b = *p;
  foo (&b, 0);
}

void
ftest3 (void)
{
  struct S a;
  memset (&a.s, '\0', sizeof (a.s));
  foo (&a, 0);
  struct S b = a;
  foo (&b, 1);
  *p = a;
  *p = b;
  foo (&b, 0);
}
#endif

void
testTortureExecute (void)
{
#if 0
  ftest1 ();
  ftest2 ();
  ftest3 ();
  return 0;
#endif
}
