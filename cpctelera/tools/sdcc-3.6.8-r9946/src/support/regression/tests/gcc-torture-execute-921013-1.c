/*
   921013-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void f(int *d, float *x, float *y,int n)
{
  while(n--){*d++=*x++==*y++;}
}

void
testTortureExecute (void)
{
  int r[4]={2,2,2,2};
  float a[]={5,1,3,5};
  float b[]={2,4,3,0};

  f(r,a,b,4);
  ASSERT ((a[0]==b[0]) == r[0]);
  ASSERT ((a[1]==b[1]) == r[1]);
  ASSERT ((a[2]==b[2]) == r[2]);
  ASSERT ((a[3]==b[3]) == r[3]);
  return;
}
