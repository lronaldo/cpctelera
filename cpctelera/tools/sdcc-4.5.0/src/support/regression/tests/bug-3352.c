/* bug-3352.c
   A check for a division by 0 assumed the divisor to have integer type, resulting in a wrong value for compile-time divisions by float values close to 0. There was also a divide-by-zero wanring emitted.
 */

#include <testfwk.h>

#pragma disable_warning 85

float m(int argc, char **argv)
{
   int mx = 384, x0 = 1, x1 = (mx - 1) - 1;
   int my = 192, y0 = 1, y1 = (my - 1) - 8;
   float w1 = -0.866, w3 = -0.791, w2 = -0.24;
   float ar = (float)(my*4)/(float)(mx*3);
   float w4 = (w3 - w1)*(float)(y1 - y0)/(float)(x1 - x0)/ar + w2;


   return w4;
}

void
testBug (void)
{
	ASSERT (m(0, 0) < 0);
}

