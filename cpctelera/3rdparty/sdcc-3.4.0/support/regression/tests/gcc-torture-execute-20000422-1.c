/*
   20000422-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int ops[13] =
{
  11, 12, 46, 3, 2, 2, 3, 2, 1, 3, 2, 1, 2
};

int correct[13] = 
{
  46, 12, 11, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1
};

int num = 13;

void
testTortureExecute (void)
{
  int i;

  for (i = 0; i < num; i++)
    {
      int j;

      for (j = num - 1; j > i; j--)
        {
          if (ops[j-1] < ops[j])
            {
              int op = ops[j];
              ops[j] = ops[j-1];
              ops[j-1] = op;
            }
        }
    }


  for (i = 0; i < num; i++)
    if (ops[i] != correct[i])
      ASSERT (0);

  return;
}

