/*
va-arg-23.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#pragma disable_warning 85

/* PR 9700 */
/* Alpha got the base address for the va_list incorrect when there was
   a structure that was passed partially in registers and partially on
   the stack.  */

#include <stdarg.h>

struct two { long x, y; };

void foo(int a, int b, int c, int d, int e, struct two f, int g, ...)
{
  va_list args;
  int h;

  va_start(args, g);
  h = va_arg(args, int);
  if (g != 1 || h != 2)
    ASSERT (0);
}

void
testTortureExecute (void)
{
  struct two t = { 0, 0 };
  foo(0, 0, 0, 0, 0, t, 1, 2);
  return;
}
