/*
   pr64756.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR rtl-optimization/64756 */

int a, *tmp, **c = &tmp;
volatile int d;
static int *volatile *e = &tmp;
unsigned int f;

static void
fn1 (int *p)
{
  int g;
  for (; f < 1; f++)
    for (g = 1; g >= 0; g--)
      {
	d || d;
	*c = p;

	if (tmp != &a)
	  ASSERT (0);

	*e = 0;
      }
}

void
testTortureExecute (void)
{
  fn1 (&a);
  return;
}
