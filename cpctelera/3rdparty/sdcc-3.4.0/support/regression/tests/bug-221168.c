/* bug-221168.c
 */
#include <testfwk.h>

__xdata static char x[10][20];

__xdata char *
getAddrOfCell (unsigned char y, unsigned char z)
{
  return &x[y][z];
}

static void
testMultiDimensionalAddress (void)
{
  ASSERT (getAddrOfCell (5, 6) == (char __xdata *)x + 106);
}
