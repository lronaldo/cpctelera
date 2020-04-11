/*
   pr60822.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR rtl-optimization/63659 */

int a, b, c, *d = &b, g, h, i;
unsigned char e;
char f;

void
testTortureExecute (void)
{
  while (a)
    {
      for (a = 0; a; a++)
	for (; c; c++)
	  ;
      if (i)
	break;
    }
#if 0 // Enable when SDCC supports intermingled code/declarations
  char j = c, k = -1, l;
  l = g = j >> h;
  f = l == 0 ? k : k % l;
  e = 0 ? 0 : f;
  *d = e;

  if (b != 255)
    ASSERT (0);
#endif
  return;
}
