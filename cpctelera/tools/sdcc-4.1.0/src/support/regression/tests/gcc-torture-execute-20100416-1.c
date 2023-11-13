/*
20100416-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

int
movegt(int x, int y, long long a)
{
  int i;
  int ret = 0;
  for (i = 0; i < y; i++)
    {
      if (a >= (long long) 0xf000000000000000LL)
	ret = x;
      else
	ret = y;
    }
  return ret;
}

#ifndef __SDCC_pdk14 // Lack of memory
struct test
{
  long long val;
  int ret;
} tests[] = {
  { 0xf000000000000000LL, -1 },
  { 0xefffffffffffffffLL, 1 },
  { 0xf000000000000001LL, -1 },
  { 0x0000000000000000LL, -1 },
  { 0x8000000000000000LL, 1 },
};
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  int i;
  for (i = 0; i < sizeof (tests) / sizeof (tests[0]); i++)
    {
      if (movegt (-1, 1, tests[i].val) != tests[i].ret)
	ASSERT (0);
    }
  return;
#endif
}
