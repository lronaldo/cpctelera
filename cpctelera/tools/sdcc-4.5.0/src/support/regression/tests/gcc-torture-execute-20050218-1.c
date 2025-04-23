/*
   20050218-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

#if !defined(__SDCC_pdk14) // Lack of memory

/* PR tree-optimization/19828 */

const char *a[16] = { "a", "bc", "de", "fgh" };

int
foo (char *x, const char *y, size_t n)
{
  size_t i, j = 0;
  for (i = 0; i < n; i++)
    {
      if (strncmp (x + j, a[i], strlen (a[i])) != 0)
        return 2;
      j += strlen (a[i]);
      if (y)
        j += strlen (y);
    }
  return 0;
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) // Lack of memory
  if (foo ("abcde", (const char *) 0, 3) != 0)
    ASSERT (0);
  return;
#endif
}
