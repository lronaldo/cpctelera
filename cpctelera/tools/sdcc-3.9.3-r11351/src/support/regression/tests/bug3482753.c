/*
   bug3482753.c
*/

#include <testfwk.h>

struct {char a;} x;
volatile char d = 7;

int f(void)
{
  char t = d;
  
  x.a = t;
  t = x.a + 2;
  d = t;
  t = x.a + 3;  /* bug: x.a was optimized to t, dispite redefinition of t */
  return t;
}

void testBug(void)
{
  ASSERT(f() == 10);
}
