/*
   pr70566.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#pragma disable_warning 85

/* PR target/70566.  */

#define NULL 0

struct mystruct
{
  unsigned int f1 : 1;
  unsigned int f2 : 1;
  unsigned int f3 : 1;
};

void
myfunc (int a, void *b)
{
}
int
myfunc2 (void *a)
{
  return 0;
}

static void
set_f2 (struct mystruct *user, int f2)
{
  if (user->f2 != f2)
    myfunc (myfunc2 (NULL), NULL);
  else
    ASSERT (0);
}

void
foo (void *data)
{
  struct mystruct *user = data;
  if (!user->f2)
    set_f2 (user, 1);
}

void
testTortureExecute (void)
{
  struct mystruct a;
  a.f1 = 1;
  a.f2 = 0;
  foo (&a);
  return;
}
