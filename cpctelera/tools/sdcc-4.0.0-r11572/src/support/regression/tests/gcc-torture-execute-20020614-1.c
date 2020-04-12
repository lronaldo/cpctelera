/*
   20020614-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR c/6677 */
/* Verify that GCC doesn't perform illegal simplifications
   when folding constants.  */

#include <limits.h>

void testTortureExecute (void)
{
  int i;
  signed char j;
  unsigned char k;

  i = SCHAR_MAX;

  j = ((signed char) (i << 1)) / 2;

  if (j != -1)
    ASSERT (0);

  j = ((signed char) (i * 2)) / 2;

  if (j != -1)
    ASSERT (0);

  i = UCHAR_MAX;

  k = ((unsigned char) (i << 1)) / 2;

  if (k != UCHAR_MAX/2)
    ASSERT (0);

  k = ((unsigned char) (i * 2)) / 2;

  if (k != UCHAR_MAX/2)
    ASSERT (0);

  return;
}
