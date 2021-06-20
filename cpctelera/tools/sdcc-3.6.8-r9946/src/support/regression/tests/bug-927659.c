/* bug-927659.c

   double processing resp. reversing of params
*/

#include <testfwk.h>
#include <stdarg.h>
#include <stdio.h>

unsigned char
foo(unsigned char a, ...) __reentrant
{
  va_list argptr;
  unsigned char b;

  va_start (argptr, a);
  b = va_arg (argptr, int);
  va_end (argptr);

  return b;
}

unsigned char
bar(unsigned char a, unsigned char b) __reentrant
{
  return b / a;
}

void
testReverse(void)
{
  ASSERT(foo (0, bar (1, 2)) == 2);
}

/*************************************************************/

void
testAddFunc(void)
{
  char buf[5];
  unsigned char count = 0;

  count += sprintf (buf, "%d", 5);
  ASSERT(count == 1 &&
         buf[0] == '5' &&
         buf[1] == '\0');
}
