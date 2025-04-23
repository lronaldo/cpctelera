/*
memcpy-bi.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 85
#endif

#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
/* Test builtin-memcpy (which may emit different code for different N).  */
#include <string.h>

#define TESTSIZE 80

char src[TESTSIZE];
char dst[TESTSIZE];

void
check (char *test, char *match, int n)
{
  if (memcmp (test, match, n))
    ASSERT (0);
}

#define TN(n) \
{ memset (dst, 0, n); memcpy (dst, src, n); check (dst, src, n); }
#define T(n) \
TN (n) \
TN ((n) + 1) \
TN ((n) + 2) \
TN ((n) + 3)
#endif

void
testTortureExecute (void)
{
#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  int i,j;

  for (i = 0; i < sizeof (src); ++i)
      src[i] = 'a' + i % 26;

  T (0);
  T (4);
  T (8);
  T (12);
  T (16);
  T (20);
  T (24);
  T (28);
  T (32);
  T (36);
  T (40);
  T (44);
  T (48);
  T (52);
  T (56);
  T (60);
  T (64);
  T (68);
  T (72);
  T (76);
#endif
  return;
}
