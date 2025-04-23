/*
   20010518-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Leaf functions with many arguments.  */

#if !defined(__SDCC_pic14) // Pseudo-stack size limit
int
add (int a,
    int b,
    int c,
    int d,
    int e,
    int f,
    int g,
    int h,
    int i,
    int j,
    int k,
    int l,
    int m)
{
  return a+b+c+d+e+f+g+h+i+j+k+l+m;
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pic14) // Pseudo-stack size limit
  if (add (1,2,3,4,5,6,7,8,9,10,11,12,13) != 91)
    ASSERT (0);
#endif

  return;
}

