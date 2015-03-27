/*
   981001-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#define NG   0x100L

unsigned long flg = 0;

long sub (int n) __reentrant
{
  int a, b ;

  if (n >= 2)
    {
      if (n % 2 == 0)
	{
	  a = sub (n / 2);
	  
	  return (a + 2 * sub (n / 2 - 1)) * a;
	}
      else
	{
	  a = sub (n / 2 + 1);
	  b = sub (n / 2);
	  
	  return a * a + b * b;
	}
    }
  else 
    return (long) n;
}

void
testTortureExecute (void)
{
  if (sub (30) != 832040L)
    flg |= NG;

  if (flg)
    ASSERT (0);
  
  return;
}

