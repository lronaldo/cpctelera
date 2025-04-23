/*
   pr60062.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

#include <string.h>

/* PR target/60062 */

int a;

static void
foo (const char *p1, int p2)
{
  (void)p2;
  ASSERT (strcmp (p1, "hello") == 0);
}

static void
bar (const char *p1)
{
  ASSERT (strcmp (p1, "hello") == 0);
}

void
testTortureExecute (void)
{
  foo ("hello", a);
  bar ("hello");
}
