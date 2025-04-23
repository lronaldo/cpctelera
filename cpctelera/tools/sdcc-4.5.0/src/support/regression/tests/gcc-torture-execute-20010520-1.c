/*
   20010520-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

static unsigned int expr_hash_table_size = 1;

void
testTortureExecute (void)
{
  int del = 1;
  unsigned int i = 0;

  if (i < expr_hash_table_size && del)
    return;
  ASSERT (0);
}

