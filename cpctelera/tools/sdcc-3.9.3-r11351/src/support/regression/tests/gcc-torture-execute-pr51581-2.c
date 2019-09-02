/*
   pr51581-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

/* PR tree-optimization/51581 */
/* { dg-require-effective-target int32plus } */

#ifndef __SDCC_pdk14 // Lack of memory

#define N 4
int a[N], c[N];
unsigned int b[N], d[N];

void
f1 (void)
{
  int i;
  for (i = 0; i < N; i++)
    c[i] = a[i] % 3;
}

void
f2 (void)
{
  int i;
  for (i = 0; i < N; i++)
    d[i] = b[i] % 3;
}

void
f3 (void)
{
  int i;
  for (i = 0; i < N; i++)
    c[i] = a[i] % 18;
}

void
f4 (void)
{
  int i;
  for (i = 0; i < N; i++)
    d[i] = b[i] % 18;
}

void
f5 (void)
{
  int i;
  for (i = 0; i < N; i++)
    c[i] = a[i] % 19;
}

void
f6 (void)
{
  int i;
  for (i = 0; i < N; i++)
    d[i] = b[i] % 19;
}

#if __SIZEOF_INT__ == 4 && __SIZEOF_LONG_LONG__ == 8
void
f7 (void)
{
  int i;
  for (i = 0; i < N; i++)
    {
      int x = (int) ((unsigned long long) (a[i] * 0x55555556LL) >> 32) - (a[i] >> 31);
      c[i] = a[i] - x * 3;
    }
}

void
f8 (void)
{
  int i;
  for (i = 0; i < N; i++)
    {
      unsigned int x = ((unsigned int) ((b[i] * 0xaaaaaaabULL) >> 32) >> 1);
      d[i] = b[i] - x * 3;
    }
}

void
f9 (void)
{
  int i;
  for (i = 0; i < N; i++)
    {
      int x = (((int) ((unsigned long long) (a[i] * 0x38e38e39LL) >> 32)) >> 2) - (a[i] >> 31);
      c[i] = a[i] - x * 18;
    }
}

void
f10 (void)
{
  int i;
  for (i = 0; i < N; i++)
    {
      unsigned int x = (unsigned int) ((b[i] * 0x38e38e39ULL) >> 32) >> 2;
      d[i] = b[i] - x * 18;
    }
}

void
f11 (void)
{
  int i;
  for (i = 0; i < N; i++)
    {
      int x = (((int) ((unsigned long long) (a[i] * 0x6bca1af3LL) >> 32)) >> 3) - (a[i] >> 31);
      c[i] = a[i] - x * 19;
    }
}

void
f12 (void)
{
  int i;
  for (i = 0; i < N; i++)
    {
      unsigned int tmp = (b[i] * 0xaf286bcbULL) >> 32;
      unsigned int x = (((b[i] - tmp) >> 1) + tmp) >> 4;
      d[i] = b[i] - x * 19;
    }
}
#endif
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  int i;
  for (i = 0; i < N; i++)
    {
#ifndef PORT_HOST
      __asm;
      __endasm;
#endif
      a[i] = i - N / 2;
      b[i] = i;
    }
  a[0] = -INT_MAX - 1;
  a[1] = -INT_MAX;
  a[N - 1] = INT_MAX;
  b[N - 1] = ~0;
  f1 ();
  f2 ();
  for (i = 0; i < N; i++)
    if (c[i] != a[i] % 3 || d[i] != b[i] % 3)
      ASSERT (0);
  f3 ();
  f4 ();
  for (i = 0; i < N; i++)
    if (c[i] != a[i] % 18 || d[i] != b[i] % 18)
      ASSERT (0);
  f5 ();
  f6 ();
  for (i = 0; i < N; i++)
    if (c[i] != a[i] % 19 || d[i] != b[i] % 19)
      ASSERT (0);
#if __SIZEOF_INT__ == 4 && __SIZEOF_LONG_LONG__ == 8
  f7 ();
  f8 ();
  for (i = 0; i < N; i++)
    if (c[i] != a[i] % 3 || d[i] != b[i] % 3)
      ASSERT (0);
  f9 ();
  f10 ();
  for (i = 0; i < N; i++)
    if (c[i] != a[i] % 18 || d[i] != b[i] % 18)
      ASSERT (0);
  f11 ();
  f12 ();
  for (i = 0; i < N; i++)
    if (c[i] != a[i] % 19 || d[i] != b[i] % 19)
      ASSERT (0);
#endif
#endif
}
