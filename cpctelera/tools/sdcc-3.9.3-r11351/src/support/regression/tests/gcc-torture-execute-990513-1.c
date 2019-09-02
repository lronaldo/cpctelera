/*
   990513-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

void foo (int *BM_tab, int j)
{
  int *BM_tab_base;

  BM_tab_base = BM_tab;
  BM_tab += 0400;
  while (BM_tab_base != BM_tab)
    {
      *--BM_tab = j;
      *--BM_tab = j;
      *--BM_tab = j;
      *--BM_tab = j;
    }
}

void
testTortureExecute (void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  int BM_tab[0400];
  memset (BM_tab, 0, sizeof (BM_tab));
  foo (BM_tab, 6);
  if (BM_tab[0] != 6)
    ASSERT (0);
  return;
#endif
}

