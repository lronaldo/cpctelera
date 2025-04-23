/*
strct-varg-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#include <stdarg.h>

struct s { int x, y; };

void
f (int attr, ...)
{
  struct s va_values;
  va_list va;

  va_start (va, attr);

  ASSERT(attr == 2);

  va_values = va_arg (va, struct s);
  ASSERT(!(va_values.x != 0xaaaa || va_values.y != 0x5555));

  attr = va_arg (va, int);
  ASSERT(attr == 3);

  va_values = va_arg (va, struct s);
  ASSERT(!(va_values.x != 0xffff || va_values.y != 0x1111));
  va_end (va);
}

void
testTortureExecute (void)
{
  struct s a, b;

  a.x = 0xaaaa;
  a.y = 0x5555;
  b.x = 0xffff;
  b.y = 0x1111;

  f (2, a, 3, b);
  return;
}
