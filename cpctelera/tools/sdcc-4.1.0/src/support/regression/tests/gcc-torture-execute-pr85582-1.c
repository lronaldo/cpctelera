/*
   pr85582-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

/* PR target/85582 */

int a, b, d = 2, e;
long long c = 1;

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !(defined(__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of memory
  int g = 6;
L1:
  e = d;
  if (a)
    goto L1;
  g--;
  int i = c >> ~(~e | ~g);
L2:
  c = (b % c) * i;
  if (!e)
    goto L2;
  return;
#endif
}
