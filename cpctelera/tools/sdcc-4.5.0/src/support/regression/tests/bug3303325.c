/*
   bug3303325.c
*/

#include <testfwk.h>

unsigned char a;
void f(unsigned char b)
{	
  a = b;
}

void
testBug (void)
{
  a = 1;
  f(2);
  ASSERT (a == 2);
}

