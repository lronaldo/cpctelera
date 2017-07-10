/*
   bug-2253.c was a bug in operand handling of operands that are outside the stack-pointer-offset range in code generation for wide division in the stm8 port.
 */

#include <testfwk.h>

/* Reduce array size for ports that can't handle large local variables */
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic16)
 #define ARRAYSIZE 255
#else
 #define ARRAYSIZE 2
#endif

char ad(char *p)
{
  p;
}

unsigned int ss(unsigned int c, unsigned int d)
{
  char s1[ARRAYSIZE];
  ad(s1);
  return c / d;
}

void testBug(void)
{
  ASSERT(ss(4, 2) == 2);
}

