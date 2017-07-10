/*
   20080604-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct barstruct { char const* some_string; } x;

void
foo(void)
{
  if (!x.some_string)
    ASSERT (0);
}
void baz(int b)
{
  struct barstruct bar;
  struct barstruct* barptr;
  if (b)
    barptr = &bar;
  else
    {
      barptr = &x + 1;
      barptr = barptr - 1;
    }
  barptr->some_string = "Everything OK";
  foo();
  barptr->some_string = "Everything OK";
}

void
testTortureExecute (void)
{
  x.some_string = (void *)0;
  baz(0);
  if (!x.some_string)
    ASSERT (0);
  return;
}

