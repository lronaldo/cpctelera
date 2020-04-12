/* bug-221168.c
 */
#include <testfwk.h>

#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Lack of memory
__xdata static char x[10][20];

__xdata char *
getAddrOfCell (unsigned char y, unsigned char z)
{
  return &x[y][z];
}
#endif

static void
testMultiDimensionalAddress (void)
{
#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Lack of memory
  ASSERT (getAddrOfCell (5, 6) == (char __xdata *)x + 106);
#endif
}

