/*
   bug-3027.c - loop induction left iTemp marked used in a deleted iCode,
                causing segfault in loop narrowing
 */

#include <testfwk.h>

void f(int **p, int x)
{
  for (unsigned int i=0; i<100; i++)
    *p[i] = x;
}

void testBug(void)
{
}

