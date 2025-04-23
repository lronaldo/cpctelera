/*
   20000726-1.c from the execute part of the gcc torture tests.
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
  short kx;
  short kz;
};

static struct adjust_template adjust = {0, 0, 1, 1};

void
testTortureExecute (void)
{
  short x = 1, y = 1;

  adjust_xy (&x, &y);

  if (x != 1)
    ASSERT (0);

  return;
}

void
adjust_xy (short *x, short *y)
{
  *x = adjust.kx_x * *x + adjust.kx_y * *y + adjust.kx;
}

