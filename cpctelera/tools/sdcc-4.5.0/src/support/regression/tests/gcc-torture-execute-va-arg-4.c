/*
va-arg-4.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* On the i960 any arg bigger than 16 bytes causes all subsequent args
   to be passed on the stack.  We test this.  */

#include <stdarg.h>

typedef struct {
  char a[32];
} big;

void
f (big x, char *s, ...)
{
  va_list ap;

  if (x.a[0] != 'a' || x.a[1] != 'b' || x.a[2] != 'c')
    ASSERT (0);
  va_start (ap, s);
  if (va_arg (ap, int) != 42)
    ASSERT (0);
  if (va_arg (ap, int) != 'x')
    ASSERT (0);
  if (va_arg (ap, int) != 0)
    ASSERT (0);
  va_end (ap);
}

void
testTortureExecute (void)
{
  static big x = { "abc" };

  f (x, "", 42, 'x', 0);
  return;
}
