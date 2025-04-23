/*
   modulong.c
   A modulo algrithm. The point of this test is that it triggers otherwise untested peephole rules.
 */

#include <testfwk.h>

#define MSB_SET(x) ((x >> (8*sizeof(x)-1)) & 1)

unsigned long
modulong (unsigned long a, unsigned long b)
{
  unsigned char count = 0;

  while (!MSB_SET(b))
  {
     b <<= 1;
     if (b > a)
     {
        b >>=1;
        break;
     }
     count++;
  }
  do
  {
    if (a >= b)
      a -= b;
    b >>= 1;
  }
  while (count--);

  return a;
}

void
testMod(void)
{
	ASSERT (modulong (42ul, 23ul) == 42ul % 23ul);
}

