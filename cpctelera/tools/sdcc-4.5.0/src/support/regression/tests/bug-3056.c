/*
   bug-3056.c - function name not promoted to function pointer
                inside conditional operator
 */

#include <testfwk.h>

unsigned char x;

void f1(void)
{
  x = 1;
}

void f2(void)
{
  x = 2;
}

void bug(int i)
{
  (i ? f1 : f2)();
}

void testBug(void)
{
  ASSERT(x == 0);
  bug(1);
  ASSERT(x == 1);
  bug(0);
  ASSERT(x == 2);
}

