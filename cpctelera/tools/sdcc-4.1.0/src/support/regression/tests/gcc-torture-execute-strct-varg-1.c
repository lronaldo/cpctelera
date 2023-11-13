/*
strct-varg-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#include <stdarg.h>

struct s { int x, y; };

#if 0 // TODO: enable when SDCC supports passing struct arguments!
f (int attr, ...)
{
  struct s va_values;
  va_list va;
  int i;

  va_start (va, attr);

  if (attr != 2)
    ASSERT (0);

  va_values = va_arg (va, struct s);
  if (va_values.x != 0xaaaa || va_values.y != 0x5555)
    ASSERT (0);

  attr = va_arg (va, int);
  if (attr != 3)
    ASSERT (0);

  va_values = va_arg (va, struct s);
  if (va_values.x != 0xffff || va_values.y != 0x1111)
    ASSERT (0);
  va_end (va);
}
#endif

void
testTortureExecute (void)
{
#if 0 // TODO: enable when SDCC supports passing struct arguments!
  struct s a, b;

  a.x = 0xaaaa;
  a.y = 0x5555;
  b.x = 0xffff;
  b.y = 0x1111;

  f (2, a, 3, b);
#endif
  return;
}
