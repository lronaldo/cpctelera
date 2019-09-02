/*
   loop-5.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

static int ap(int i);
static void testit(void){
  int ir[4] = {0,1,2,3};
  int ix,n,m;
  n=1; m=3;
  for (ix=1;ix<=4;ix++) {
    if (n == 1) m = 4;
    else        m = n-1;
    ap(ir[n-1]);
    n = m;
  }
}

static int t = 0;
static int a[4];

static int ap(int i){
  if (t > 3)
    ASSERT (0);
  a[t++] = i;
  return 1;
}

void
testTortureExecute (void)
{
  testit();
  if (a[0] != 0)
    ASSERT (0);
  if (a[1] != 3)
    ASSERT (0);
  if (a[2] != 2)
    ASSERT (0);
  if (a[3] != 1)
    ASSERT (0);
  return;
}
