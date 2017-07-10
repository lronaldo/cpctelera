/*
   bug1198642.c
*/

#include <testfwk.h>

void
test_cse_generic_ptr (void)
{
#if defined(__SDCC_mcs51)
  volatile void *p1;
  volatile void *p2;

  p1 = (__data char *)1;
  p2 = (__idata char *)1;
  ASSERT (p1 == p2);

  p1 = (__data char *)1;
  p2 = (__xdata char *)1;
  ASSERT (p1 != p2);

  p1 = (__data char *)1;
  p2 = (__idata char *)2;
  ASSERT (p1 != p2);
#endif
}
