/*
cmpdi-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#define F 140
#define T 13

#if !(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM))) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory

#if 0 // Enable when SDCC support K&R-style
feq (x, y)
     long long int x;
     long long int y;
{
  if (x == y)
    return T;
  else
    return F;
}

fne (x, y)
     long long int x;
     long long int y;
{
  if (x != y)
    return T;
  else
    return F;
}

flt (x, y)
     long long int x;
     long long int y;
{
  if (x < y)
    return T;
  else
    return F;
}

fge (x, y)
     long long int x;
     long long int y;
{
  if (x >= y)
    return T;
  else
    return F;
}

fgt (x, y)
     long long int x;
     long long int y;
{
  if (x > y)
    return T;
  else
    return F;
}

fle (x, y)
     long long int x;
     long long int y;
{
  if (x <= y)
    return T;
  else
    return F;
}

fltu (x, y)
     unsigned long long int x;
     unsigned long long int y;
{
  if (x < y)
    return T;
  else
    return F;
}

fgeu (x, y)
     unsigned long long int x;
     unsigned long long int y;
{
  if (x >= y)
    return T;
  else
    return F;
}

fgtu (x, y)
     unsigned long long int x;
     unsigned long long int y;
{
  if (x > y)
    return T;
  else
    return F;
}

fleu (x, y)
     unsigned long long int x;
     unsigned long long int y;
{
  if (x <= y)
    return T;
  else
    return F;
}
#endif
long long args[] =
{
  0LL,
  1LL,
  -1LL,
  0x7fffffffffffffffLL,
  0x8000000000000000LL,
  0x8000000000000001LL,
  0x1A3F237394D36C58LL,
  0x93850E92CAAC1B04LL
};

int correct_results[] =
{
  T, F, F, T, F, T, F, T, F, T,
  F, T, T, F, F, T, T, F, F, T,
  F, T, F, T, T, F, T, F, F, T,
  F, T, T, F, F, T, T, F, F, T,
  F, T, F, T, T, F, T, F, F, T,
  F, T, F, T, T, F, T, F, F, T,
  F, T, T, F, F, T, T, F, F, T,
  F, T, F, T, T, F, T, F, F, T,
  F, T, F, T, T, F, F, T, T, F,
  T, F, F, T, F, T, F, T, F, T,
  F, T, F, T, T, F, T, F, F, T,
  F, T, T, F, F, T, T, F, F, T,
  F, T, F, T, T, F, T, F, F, T,
  F, T, F, T, T, F, T, F, F, T,
  F, T, T, F, F, T, T, F, F, T,
  F, T, F, T, T, F, T, F, F, T,
  F, T, T, F, F, T, F, T, T, F,
  F, T, T, F, F, T, F, T, T, F,
  T, F, F, T, F, T, F, T, F, T,
  F, T, T, F, F, T, F, T, T, F,
  F, T, F, T, T, F, F, T, T, F,
  F, T, F, T, T, F, F, T, T, F,
  F, T, T, F, F, T, F, T, T, F,
  F, T, F, T, T, F, F, T, T, F,
  F, T, F, T, T, F, F, T, T, F,
  F, T, F, T, T, F, F, T, T, F,
  F, T, F, T, T, F, T, F, F, T,
  T, F, F, T, F, T, F, T, F, T,
  F, T, F, T, T, F, T, F, F, T,
  F, T, F, T, T, F, T, F, F, T,
  F, T, F, T, T, F, F, T, T, F,
  F, T, F, T, T, F, T, F, F, T,
  F, T, T, F, F, T, F, T, T, F,
  F, T, T, F, F, T, F, T, T, F,
  F, T, T, F, F, T, T, F, F, T,
  F, T, T, F, F, T, F, T, T, F,
  T, F, F, T, F, T, F, T, F, T,
  F, T, T, F, F, T, T, F, F, T,
  F, T, T, F, F, T, F, T, T, F,
  F, T, T, F, F, T, T, F, F, T,
  F, T, T, F, F, T, F, T, T, F,
  F, T, T, F, F, T, F, T, T, F,
  F, T, T, F, F, T, T, F, F, T,
  F, T, T, F, F, T, F, T, T, F,
  F, T, F, T, T, F, F, T, T, F,
  T, F, F, T, F, T, F, T, F, T,
  F, T, T, F, F, T, F, T, T, F,
  F, T, T, F, F, T, T, F, F, T,
  F, T, F, T, T, F, F, T, T, F,
  F, T, F, T, T, F, F, T, T, F,
  F, T, F, T, T, F, T, F, F, T,
  F, T, T, F, F, T, T, F, F, T,
  F, T, F, T, T, F, T, F, F, T,
  F, T, F, T, T, F, T, F, F, T,
  T, F, F, T, F, T, F, T, F, T,
  F, T, F, T, T, F, T, F, F, T,
  F, T, T, F, F, T, F, T, T, F,
  F, T, T, F, F, T, F, T, T, F,
  F, T, T, F, F, T, T, F, F, T,
  F, T, T, F, F, T, F, T, T, F,
  F, T, F, T, T, F, F, T, T, F,
  F, T, F, T, T, F, F, T, T, F,
  F, T, T, F, F, T, F, T, T, F,
  T, F, F, T, F, T, F, T, F, T
};
#endif

void
testTortureExecute (void)
{
#if 0
  int i, j, *res = correct_results;

  for (i = 0; i < 8; i++)
    {
      long long arg0 = args[i];
      for (j = 0; j < 8; j++)
	{
	  long long arg1 = args[j];

	  if (feq (arg0, arg1) != *res++)
	    ASSERT (0);
	  if (fne (arg0, arg1) != *res++)
	    ASSERT (0);
	  if (flt (arg0, arg1) != *res++)
	    ASSERT (0);
	  if (fge (arg0, arg1) != *res++)
	    ASSERT (0);
	  if (fgt (arg0, arg1) != *res++)
	    ASSERT (0);
	  if (fle (arg0, arg1) != *res++)
	    ASSERT (0);
	  if (fltu (arg0, arg1) != *res++)
	    ASSERT (0);
	  if (fgeu (arg0, arg1) != *res++)
	    ASSERT (0);
	  if (fgtu (arg0, arg1) != *res++)
	    ASSERT (0);
	  if (fleu (arg0, arg1) != *res++)
	    ASSERT (0);
	}
    }
#endif
  return;
}
