/*
   20060930-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#pragma disable_warning 85

/* PR rtl-optimization/28096 */
/* Origin: Jan Stein <jan@gatespacetelematics.com> */

int bar (int, int);
int bar (int a, int b)
{
  if (b != 1)
    ASSERT (0);
}

void foo(int, int);
void foo (int e, int n)
{
  int i, bb2, bb5;

  if (e > 0)
    e = -e;

  for (i = 0; i < n; i++)
    {
      if (e >= 0)
	{
	  bb2 = 0;
	  bb5 = 0;
	}
      else
	{
	  bb5 = -e;
	  bb2 = bb5;
	}

      bar (bb5, bb2);
    }
}

void testTortureExecute (void)
{
  foo (1, 1);
  return;
}
