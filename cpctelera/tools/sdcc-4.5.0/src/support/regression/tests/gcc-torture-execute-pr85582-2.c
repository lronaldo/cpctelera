/*
   pr85582-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#include <limits.h>

/* PR target/85582 */

typedef long long S;
typedef unsigned long long U;

S
f1 (S x, int y)
{
  x = x << (y & 5);
  x += y;
  return x;
}

S
f2 (S x, int y)
{
  x = x >> (y & 5);
  x += y;
  return x;
}

U
f3 (U x, int y)
{
  x = x >> (y & 5);
  x += y;
  return x;
}

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) // Lack of data memory
  S a = (S) 1 << (sizeof (S) * CHAR_BIT - 7);
  S b = f1 (a, 12);
  if (b != ((S) 1 << (sizeof (S) * CHAR_BIT - 3)) + 12)
    ASSERT (0);
  S c = (U) 1 << (sizeof (S) * CHAR_BIT - 1);
  S d = f2 (c, 12);
  if ((U) d != ((U) 0x1f << (sizeof (S) * CHAR_BIT - 5)) + 12)
    ASSERT (0);
  U e = (U) 1 << (sizeof (U) * CHAR_BIT - 1);
  U f = f3 (c, 12);
  if (f != ((U) 1 << (sizeof (U) * CHAR_BIT - 5)) + 12)
    ASSERT (0);
  return;
#endif
}

