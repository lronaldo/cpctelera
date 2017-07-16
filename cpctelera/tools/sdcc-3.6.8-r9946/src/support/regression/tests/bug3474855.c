/*
   bug3474855.c
*/

#include <testfwk.h>

float mult(float x)
{
  return 0.8 * x;
}

void
testBug (void)
{
  unsigned char i = 64;
  float result = 1e-38;

  do
    {
      ASSERT (result < 1.5e-38);
      result = mult(result);    //make denormalized float
    }
  while (--i);
  ASSERT (result < 1e-44);
}
