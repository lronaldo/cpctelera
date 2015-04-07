/*
   bug2942343.c
 */
#pragma std_sdcc99

#include <testfwk.h>

const unsigned char x[3] = {255, 256, 257}; //gives a warning about out-of-range values.
      unsigned char y[3] = {255, 256, 257}; //does not.

const _Bool foo[4] = {1, 2, 3, 4};          //creates an array of 4 _Bools which all have the value true.
      _Bool bar[4] = {1, 2, 3, 4};          //creates an array with true in foo[0], but 2,3,4 in the rest.

void testBug(void)
{
/* Test fails on big endian hosts that use a _Bool larger than char */
/* (for example, Mac PPC), so just skip this test for non-SDCC */
#ifdef __SDCC
  ASSERT(1 == *(char*)(&foo[1]));
  ASSERT(1 == *(char*)(&bar[2]));
#endif
}

