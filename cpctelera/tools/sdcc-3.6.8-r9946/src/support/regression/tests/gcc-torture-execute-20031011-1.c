/*
   20031011-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Check that MAX_EXPR and MIN_EXPR are working properly.  */

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

void
testTortureExecute (void)
{
  int ll_bitsize, ll_bitpos;
  int rl_bitsize, rl_bitpos;
  int end_bit;

  ll_bitpos = 32;  ll_bitsize = 32;
  rl_bitpos = 0;   rl_bitsize = 32;

  end_bit = MAX (ll_bitpos + ll_bitsize, rl_bitpos + rl_bitsize);
  if (end_bit != 64)
    ASSERT (0);
  end_bit = MAX (rl_bitpos + rl_bitsize, ll_bitpos + ll_bitsize);
  if (end_bit != 64)
    ASSERT (0);
  end_bit = MIN (ll_bitpos + ll_bitsize, rl_bitpos + rl_bitsize);
  if (end_bit != 32)
    ASSERT (0);
  end_bit = MIN (rl_bitpos + rl_bitsize, ll_bitpos + ll_bitsize);
  if (end_bit != 32)
    ASSERT (0);
  return;
}

