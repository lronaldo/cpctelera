/*
   bug3474855.c
*/

#include <testfwk.h>

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
float mult(float x)
{
  return 0.8 * x;
}
#endif

void
testBug (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  unsigned char i = 64;
  float result = 1e-38;

  do
    {
      ASSERT (result < 1.5e-38);
      result = mult(result);    //make denormalized float
    }
  while (--i);
  ASSERT (result < 1e-44);
#endif
}
