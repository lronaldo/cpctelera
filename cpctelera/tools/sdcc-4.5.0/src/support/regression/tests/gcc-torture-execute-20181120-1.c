/*
   20181120-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

/* PR rtl-optimization/85925 */
/* { dg-require-effective-target int32plus } */
/* Testcase by <sudi@gcc.gnu.org> */

int a, c, d;
volatile int b;
int *e = &d;

union U1 {
  unsigned f0;
  unsigned f1 : 15;
};
volatile union U1 u = { 0x4030201 };

void
testTortureExecute (void)
{
  for (c = 0; c <= 1; c++) {
    union U1 f = {0x4030201};
    if (c == 1)
      b;
    *e = f.f1;
  }

  if (d != u.f1)
    ASSERT (0);

  return;
}
