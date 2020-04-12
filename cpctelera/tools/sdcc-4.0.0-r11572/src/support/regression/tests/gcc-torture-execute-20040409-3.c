/*
   20040409-3.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

int ftest1(int x)
{
  return ~(x ^ INT_MIN);
}

unsigned int ftest1u(unsigned int x)
{
  return ~(x ^ (unsigned int)INT_MIN);
}

int ftest2(int x)
{
  return ~(x + INT_MIN);
}

unsigned int ftest2u(unsigned int x)
{
  return ~(x + (unsigned int)INT_MIN);
}

unsigned int ftest3u(unsigned int x)
{
  return ~(x - (unsigned int)INT_MIN);
}

int ftest4(int x)
{
  int y = INT_MIN;
  return ~(x ^ y);
}

unsigned int ftest4u(unsigned int x)
{
  unsigned int y = (unsigned int)INT_MIN;
  return ~(x ^ y);
}

int ftest5(int x)
{
  int y = INT_MIN;
  return ~(x + y);
}

unsigned int ftest5u(unsigned int x)
{
  unsigned int y = (unsigned int)INT_MIN;
  return ~(x + y);
}

unsigned int ftest6u(unsigned int x)
{
  unsigned int y = (unsigned int)INT_MIN;
  return ~(x - y);
}



void ftest(int a, int b)
{
  if (ftest1(a) != b)
    ASSERT (0);
  if (ftest2(a) != b)
    ASSERT (0);
  if (ftest4(a) != b)
    ASSERT (0);
  if (ftest5(a) != b)
    ASSERT (0);
}

void ftestu(unsigned int a, unsigned int b)
{
  if (ftest1u(a) != b)
    ASSERT (0);
  if (ftest2u(a) != b)
    ASSERT (0);
  if (ftest3u(a) != b)
    ASSERT (0);
  if (ftest4u(a) != b)
    ASSERT (0);
  if (ftest5u(a) != b)
    ASSERT (0);
  if (ftest6u(a) != b)
    ASSERT (0);
}


void
testTortureExecute (void)
{
#if INT_MAX == 2147483647
  ftest(0x00000000,0x7fffffff);
  ftest(0x80000000,0xffffffff);
  ftest(0x12345678,0x6dcba987);
  ftest(0x92345678,0xedcba987);
  ftest(0x7fffffff,0x00000000);
  ftest(0xffffffff,0x80000000);

  ftestu(0x00000000,0x7fffffff);
  ftestu(0x80000000,0xffffffff);
  ftestu(0x12345678,0x6dcba987);
  ftestu(0x92345678,0xedcba987);
  ftestu(0x7fffffff,0x00000000);
  ftestu(0xffffffff,0x80000000);
#endif

#if INT_MAX == 32767
  ftest(0x0000,0x7fff);
  ftest(0x8000,0xffff);
  ftest(0x1234,0x6dcb);
  ftest(0x9234,0xedcb);
  ftest(0x7fff,0x0000);
  ftest(0xffff,0x8000);

  ftestu(0x0000,0x7fff);
  ftestu(0x8000,0xffff);
  ftestu(0x1234,0x6dcb);
  ftestu(0x9234,0xedcb);
  ftestu(0x7fff,0x0000);
  ftestu(0xffff,0x8000);
#endif

  return;
}

