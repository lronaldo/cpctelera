/*
   20010123-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when supported by sdcc!

struct s
{
    int value;
    char *string;
};

void
testTortureExecute (void)
{
#if 0
  int i;
  for (i = 0; i < 4; i++)
    {
      struct s *t = & (struct s) { 3, "hey there" };
      if (t->value != 3)
	ASSERT (0);
      t->value = 4;
      if (t->value != 4)
	ASSERT (0);
    }
  return;
#endif
}

