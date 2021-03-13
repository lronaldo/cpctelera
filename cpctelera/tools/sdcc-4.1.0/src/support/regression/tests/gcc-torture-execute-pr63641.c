/*
   pr63641.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/63641 */

int
foo (unsigned char b)
{
  if (0x0 <= b && b <= 0x8)
    goto lab;
  if (b == 0x0b)
    goto lab;
  if (0x0e <= b && b <= 0x1a)
    goto lab;
  if (0x1c <= b && b <= 0x1f)
    goto lab;
  return 0;
lab:
  return 1;
}

int
bar (unsigned char b)
{
  if (0x0 <= b && b <= 0x8)
    goto lab;
  if (b == 0x0b)
    goto lab;
  if (0x0e <= b && b <= 0x1a)
    goto lab;
  if (0x3c <= b && b <= 0x3f)
    goto lab;
  return 0;
lab:
  return 1;
}

#ifndef __SDCC_pdk14 // Lack of memory
char tab1[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 };
char tab2[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 };
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  int i;

  for (i = 0; i < 256; i++)
    if (foo (i) != (i < 32 ? tab1[i] : 0))
      ASSERT (0);
  for (i = 0; i < 256; i++)
    if (bar (i) != (i < 64 ? tab2[i] : 0))
      ASSERT (0);
  return;
#endif
}
