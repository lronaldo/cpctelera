/*
   20170401-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void adjust_xy (short *, short *);

struct adjust_template
{
  short kx_x;
  short kx_y;
};

static struct adjust_template adjust = {1, 1};

void
testTortureExecute (void)
{
  short x = 1, y = 1;

  adjust_xy (&x, &y);

  if (x != 2)
    ASSERT (0);
}

void
adjust_xy (short *x, short *y)
{
  *x = adjust.kx_x * *x + adjust.kx_y * *y;
}
