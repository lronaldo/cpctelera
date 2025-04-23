/*
   pr91137.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_mcs51) && !defined(__SDCC_sm83) && !defined(__SDCC_f8) // Lack of memory
long long a;
unsigned b;
int c[70];
int d[70][70];
int e;

void f(long long *g, int p2) {
  *g = p2;
}

void fn2() {
  for (int j = 0; j < 70; j++) {
    for (int i = 0; i < 70; i++) {
      if (b)
        c[i] = 0;
      for (int l = 0; l < 70; l++)
        d[i][1] = d[l][i];
    }
    for (int k = 0; k < 70; k++)
      e = c[0];
  }
}
#endif

void
testTortureExecute (void)
{
#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_mcs51) && !defined(__SDCC_sm83) && !defined(__SDCC_f8) // Lack of memory
  b = 5;
  for (int j = 0; j < 70; ++j)
    c[j] = 2075593088;
  fn2();
  f(&a, e);
  if (a)
    ASSERT (0);
  return;
#endif
}

