/** Bug 2373
 */
#include <testfwk.h>

char func(char a)
{
  return a-1;
}

static char a;

static struct
{
  char c;
} s;

static char (* __xdata func_ptr) (char);

void
testFptr(void)
{
  char b = 10;
  a      = 10;
  s.c    = 10;
  func_ptr=func;
  
  // works as expected
  ASSERT(func_ptr(a) == 9);
  ASSERT(func_ptr(b) == 9);
  
  // error passing s.c to func
  ASSERT(func_ptr(s.c) == 9);
}
