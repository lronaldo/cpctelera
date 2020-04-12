/*
  size: 126, 127, 128, 129
 */
#include <testfwk.h>

void
spoil(char a)
{
  UNUSED(a);
}

void
spoilPtr(volatile char *p)
{
  UNUSED(p);
}

void
testStack(void)
{
  volatile char above;
  volatile char above2;
/* pic14: Bank size limit */
#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk14) && !defined(__SDCC_pic14)
  volatile char ac[{size}];
#else
  volatile char ac[{size} - 100];
#endif
  volatile char below;
  volatile char * volatile p;

  spoil(ac[0]);
  spoilPtr(&above);
  spoilPtr(&below);

  p = &above2;
  spoilPtr(p);
}
