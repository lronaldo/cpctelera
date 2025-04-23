/*
   bug-3320.c
   A variable scope issue resulting in an invalid compile-time error message.
   */

#include <testfwk.h>

char c;

void foo()
{
  char *dest = &c;
  {
    *dest++ |= 1;
  }
}

void testBug(void)
{
  c = 42;
  foo ();
  ASSERT (c == 43);
}

