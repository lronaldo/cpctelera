/*
20171008-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 85
#endif

struct S { char c1, c2, c3, c4; };

static char bar (char **p);
static struct S foo (void);

int i;

static char
bar (char **p)
{
  i = 1;
  return 0;
}

#if 0 // Enable when SDCC allows struct initialization by foo()
static struct S
foo (void)
{
  struct S ret;
  char r, s, c1, c2;
  char *p = &r;

  s = bar (&p);
  if (s)
    c2 = *p;
  c1 = 0;

  ret.c1 = c1;
  ret.c2 = c2;
  return ret;
}
#endif

void
testTortureExecute (void)
{
#if 0 // Enable when SDCC allows struct initialization by foo()
  struct S s = foo ();
  if (s.c1 != 0)
    ASSERT (0);
  return;
#endif
}

