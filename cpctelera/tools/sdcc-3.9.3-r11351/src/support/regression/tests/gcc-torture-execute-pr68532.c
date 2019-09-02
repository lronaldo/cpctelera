/*
   pr68532.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c11
#endif

/* { dg-options "-O2 -ftree-vectorize -fno-vect-cost-model" } */
/* { dg-additional-options "-fno-common" { target hppa*-*-hpux* } } */

#define SIZE 128
#if !(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM))) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Array too big to fit in memory.
unsigned short _Alignas (/*16 - not supported in SDCC*/0) in[SIZE];
#endif

int
test (unsigned short sum, unsigned short *in, int x)
{
  for (int j = 0; j < SIZE; j += 8)
    sum += in[j] * x;
  return sum;
}

void
testTortureExecute (void)
{
#if !(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM))) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)
  for (int i = 0; i < SIZE; i++)
    in[i] = i;

  if (test (0, in, 1) != 960)
    ASSERT (0);
#endif
  return;
}
