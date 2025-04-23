/*
   bug-2419.c
*/

#include <testfwk.h>

#if defined (__SDCC_mcs51) || defined (__SDCC_hc08) || defined (__SDCC_s08) || defined (__SDCC_ds390) || defined (__SDCC_ds400)
#define XDATA __xdata
#else
#define XDATA
#endif

char XDATA c0[] = "123";
char XDATA c1[] = "abc";
char XDATA *gp = c0;

void XDATA *aligned_a (void)
{
  return gp;
}

extern void XDATA *aligned_a (void);

void testBug (void)
{
  ASSERT (aligned_a () == c0);
  gp = c1;
  ASSERT (aligned_a () == c1);
}

