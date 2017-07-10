/*
   961213-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when long long constants are supported!
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
int
g (unsigned long long int *v, int n, unsigned int a[], int b)
{
  int cnt;
  *v = 0;
  for (cnt = 0; cnt < n; ++cnt)
    *v = *v * b + a[cnt];
  return n;
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_hc08) && !defined (__SDCC_s08) 
#if !defined (__SDCC_gbz80) // bug #2329
  int res;
  unsigned int ar[] = { 10, 11, 12, 13, 14 };
  unsigned long long int v;

  res = g (&v, sizeof(ar)/sizeof(ar[0]), ar, 16);
  if (v != 0xabcdeUL)
    ASSERT (0);

  return;
#endif
#endif
}

