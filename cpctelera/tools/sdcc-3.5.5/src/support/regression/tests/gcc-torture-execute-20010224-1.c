/*
   20010224-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

#include <stdint.h>

int16_t logadd (int16_t *a, int16_t *b);
void ba_compute_psd (int16_t start);

int16_t masktab[6] = { 1, 2, 3, 4, 5};
int16_t psd[6] = { 50, 40, 30, 20, 10};
int16_t bndpsd[6] = { 1, 2, 3, 4, 5};

void ba_compute_psd (int16_t start)
{
  int i,j,k;
  int16_t lastbin = 4;

  j = start; 
  k = masktab[start]; 

  bndpsd[k] = psd[j]; 
  j++; 

  for (i = j; i < lastbin; i++) { 
    bndpsd[k] = logadd(&bndpsd[k], &psd[j]);
    j++; 
  } 
}

int16_t logadd (int16_t *a, int16_t *b)
{
  return *a + *b;
}

void
testTortureExecute (void)
{
#if !defined (__SDCC_z80) && !defined (__SDCC_z180) && !defined (__SDCC_r2k) && !defined (__SDCC_r3ka) && !defined (__SDCC_gbz80) && !defined (__SDCC_stm8) // CSE bug #1968
  int i;

  ba_compute_psd (0);

  if (bndpsd[1] != 140) ASSERT (0);
  return;
#endif
}

