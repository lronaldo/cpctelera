/*
   20020413-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if 0 // TODO: Enable when long double is supported!
void ftest(long double val, int *eval)
{
  long double tmp = 1.0l;
  int i = 0;

  if (val < 0.0l)
    val = -val;

  if (val >= tmp)
    while (tmp < val)
      {
	tmp *= 2.0l;
	if (i++ >= 10)
	  ASSERT (0);
      }
  else if (val != 0.0l)
    while (val < tmp)
      {
	tmp /= 2.0l;
	if (i++ >= 10)
	  ASSERT (0);
      }

  *eval = i;
}
#endif

void testTortureExecute(void)
{
#if 0
  int eval;

  ftest(3.0, &eval);
  ftest(3.5, &eval);
  ftest(4.0, &eval);
  ftest(5.0, &eval);
  return;
#endif
}

