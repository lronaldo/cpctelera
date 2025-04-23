/*
   pr24716.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR24716, scalar evolution returning the wrong result
   for pdest.  */

int Link[] = { -1 };
int W[] = { 2 };

int f (int k, int p)
{
  int pdest, j, D1361;
  j = 0;
  pdest = 0;
  for (;;) {
    if (pdest > 2)
      do
        j--, pdest++;
      while (j > 2);

    if (j == 1)
      break;

    while (pdest > p)
      if (j == p)
        pdest++;

    do
      {
        D1361 = W[k];
        do
          if (D1361 != 0)
            pdest = 1, W[k] = D1361 = 0;
        while (p < 1);
    } while (k > 0);

    do
      {
        p = 0;
        k = Link[k];
        while (p < j)
          if (k != -1)
            pdest++, p++;
      }
    while (k != -1);
    j = 1;
  }

  /* The correct return value should be pdest (1 in the call from main).
     DOM3 is mistaken and propagates a 0 here.  */
  return pdest;
}

void
testTortureExecute (void)
{
  if (!f (0, 2))
    ASSERT (0);
  return;
}

