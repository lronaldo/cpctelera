/*
   20060905-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#ifndef __SDCC_mcs51
/* PR rtl-optimization/28386 */
/* Origin: Volker Reichelt <reichelt@gcc.gnu.org> */

volatile char s[256][3];

char g;

static void dummy(char a)
{
  g = a;
}

static int foo(void)
{
  int i, j=0;

  for (i = 0; i < 256; i++)
    if (i >= 128 && i < 256)
      {
	dummy (s[i - 128][0]);
	++j;
      }

  return j;
}
#endif

void testTortureExecute(void)
{
#ifndef __SDCC_mcs51
  if (foo () != 128)
    ASSERT (0);

  return;
#endif
}
