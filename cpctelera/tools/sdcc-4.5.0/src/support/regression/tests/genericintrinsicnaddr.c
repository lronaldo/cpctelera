/*
   C11 generic associations and their interaction with intrinsic named address spaces
*/

#include <testfwk.h>

// Only some ports have intrinsic named address spaces
#if defined(__SDCC_mcs51)
__near int *ni;
__code int *ci;
int *i;
#endif

void testGeneric(void)
{
// Only some ports have intrinsic named address spaces
#if defined(__SDCC_mcs51)
  ASSERT(_Generic(ni, default : 0, __near int *: 1, __code int* : 2) == 1);
  ASSERT(_Generic(ci, default : 0, __near int *: 1, __code int* : 2) == 2);
#endif
}

