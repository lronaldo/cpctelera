/*
   pr85582-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#include <limits.h>

/* PR target/85582 */

typedef long long S;
typedef unsigned long long U;

U
f1 (U x, int y)
{
  return x << (y & -2);
}

S
f2 (S x, int y)
{
  return x >> (y & -2);
}

U
f3 (U x, int y)
{
  return x >> (y & -2);
}

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) // Lack of data memory
  U a = (U) 1 << (sizeof (U) * CHAR_BIT - 7);
  if (f1 (a, 5) != ((U) 1 << (sizeof (S) * CHAR_BIT - 3)))
    ASSERT (0);
  S b = (U) 0x101 << (sizeof (S) * CHAR_BIT / 2 - 7);
  if (f1 (b, sizeof (S) * CHAR_BIT / 2) != (U) 0x101 << (sizeof (S) * CHAR_BIT - 7))
    ASSERT (0);
  if (f1 (b, sizeof (S) * CHAR_BIT / 2 + 2) != (U) 0x101 << (sizeof (S) * CHAR_BIT - 5))
    ASSERT (0);
  S c = (U) 1 << (sizeof (S) * CHAR_BIT - 1);
  if ((U) f2 (c, 5) != ((U) 0x1f << (sizeof (S) * CHAR_BIT - 5)))
    ASSERT (0);
  if ((U) f2 (c, sizeof (S) * CHAR_BIT / 2) != ((U) -1 << (sizeof (S) * CHAR_BIT / 2 - 1)))
    ASSERT (0);
  if ((U) f2 (c, sizeof (S) * CHAR_BIT / 2 + 2) != ((U) -1 << (sizeof (S) * CHAR_BIT / 2 - 3)))
    ASSERT (0);
  U d = (U) 1 << (sizeof (S) * CHAR_BIT - 1);
  if (f3 (c, 5) != ((U) 0x1 << (sizeof (S) * CHAR_BIT - 5)))
    ASSERT (0);
  if (f3 (c, sizeof (S) * CHAR_BIT / 2) != ((U) 1 << (sizeof (S) * CHAR_BIT / 2 - 1)))
    ASSERT (0);
  if (f3 (c, sizeof (S) * CHAR_BIT / 2 + 2) != ((U) 1 << (sizeof (S) * CHAR_BIT / 2 - 3)))
    ASSERT (0);
  return;
#endif
}

