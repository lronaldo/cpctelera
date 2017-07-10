/*
   bug-2357.c
 */

#include <testfwk.h>
#include <string.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if defined (__SDCC) && (!defined (__SDCC_mcs51) || (!defined (__SDCC_MODEL_SMALL) && !defined (__SDCC_MODEL_MEDIUM) && !defined (__SDCC_STACK_AUTO)))

short i = -1;
union {
  short sv0[126];
  short sv1[132];
  long lv0[63];
  long lv1[66];
} uv; 

short
foo2(void)
{
  return uv.sv1[132 + i];
}

long
foo3(void)
{
  return uv.lv1[66 + i];
}

short
foo4(void)
{
  return uv.sv0[126 + i];
}

long
foo5(void)
{
  return uv.lv0[63 + i];
}

#endif

void
testBug (void)
{
#if defined (__SDCC) && (!defined (__SDCC_mcs51) || (!defined (__SDCC_MODEL_SMALL) && !defined (__SDCC_MODEL_MEDIUM) && !defined (__SDCC_STACK_AUTO)))

  memset(&uv, 0x33, 128);
  memset((char *) &uv + 128, 0x55, 136);

  ASSERT (foo2() == 0x5555);
  ASSERT (foo3() == 0x55555555L);

  ASSERT (foo4() == 0x5555);
  ASSERT (foo5() == 0x55555555L);

#endif
}
