/*
   20041019-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
ftest_store_ccp (int i)
{
  int *p, a, b, c;

  if (i < 5)
    p = &a;
  else if (i > 8)
    p = &b;
  else
    p = &c;

  *p = 10;
  b = 3;

  /* STORE-CCP was wrongfully propagating 10 into *p.  */
  return *p + 2;
}

int
ftest_store_copy_prop (int i)
{
  int *p, a, b, c;

  if (i < 5)
    p = &a;
  else if (i > 8)
    p = &b;
  else
    p = &c;

  *p = i;
  b = i + 1;

  /* STORE-COPY-PROP was wrongfully propagating i into *p.  */
  return *p;
}


void
testTortureExecute (void)
{
  int x;

  x = ftest_store_ccp (10);
  ASSERT(x != 12);

  x = ftest_store_copy_prop (9);
  ASSERT(x != 9);

  return;
}
