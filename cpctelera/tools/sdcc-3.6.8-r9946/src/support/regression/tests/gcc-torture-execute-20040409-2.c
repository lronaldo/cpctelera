/*
   20040409-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

int ftest1(int x)
{
  return (x ^ INT_MIN) ^ 0x1234;
}

unsigned int ftest1u(unsigned int x)
{
  return (x ^ (unsigned int)INT_MIN) ^ 0x1234;
}

int ftest2(int x)
{
  return (x ^ 0x1234) ^ INT_MIN;
}

unsigned int ftest2u(unsigned int x)
{
  return (x ^ 0x1234) ^ (unsigned int)INT_MIN;
}

int ftest3(int x)
{
  return (x + INT_MIN) ^ 0x1234;
}

unsigned int ftest3u(unsigned int x)
{
  return (x + (unsigned int)INT_MIN) ^ 0x1234;
}

int ftest4(int x)
{
  return (x ^ 0x1234) + INT_MIN;
}

unsigned int ftest4u(unsigned int x)
{
  return (x ^ 0x1234) + (unsigned int)INT_MIN;
}

int ftest5(int x)
{
  return (x - INT_MIN) ^ 0x1234;
}

unsigned int ftest5u(unsigned int x)
{
  return (x - (unsigned int)INT_MIN) ^ 0x1234;
}

int ftest6(int x)
{
  return (x ^ 0x1234) - INT_MIN;
}

unsigned int ftest6u(unsigned int x)
{
  return (x ^ 0x1234) - (unsigned int)INT_MIN;
}

int ftest7(int x)
{
  int y = INT_MIN;
  int z = 0x1234;
  return (x ^ y) ^ z;
}

unsigned int ftest7u(unsigned int x)
{
  unsigned int y = (unsigned int)INT_MIN;
  unsigned int z = 0x1234;
  return (x ^ y) ^ z;
}

int ftest8(int x)
{
  int y = 0x1234;
  int z = INT_MIN;
  return (x ^ y) ^ z;
}

unsigned int ftest8u(unsigned int x)
{
  unsigned int y = 0x1234;
  unsigned int z = (unsigned int)INT_MIN;
  return (x ^ y) ^ z;
}

int ftest9(int x)
{
  int y = INT_MIN;
  int z = 0x1234;
  return (x + y) ^ z;
}

unsigned int ftest9u(unsigned int x)
{
  unsigned int y = (unsigned int)INT_MIN;
  unsigned int z = 0x1234;
  return (x + y) ^ z;
}

int ftest10(int x)
{
  int y = 0x1234;
  int z = INT_MIN;
  return (x ^ y) + z;
}

unsigned int ftest10u(unsigned int x)
{
  unsigned int y = 0x1234;
  unsigned int z = (unsigned int)INT_MIN;
  return (x ^ y) + z;
}

int ftest11(int x)
{
  int y = INT_MIN;
  int z = 0x1234;
  return (x - y) ^ z;
}

unsigned int ftest11u(unsigned int x)
{
  unsigned int y = (unsigned int)INT_MIN;
  unsigned int z = 0x1234;
  return (x - y) ^ z;
}

int ftest12(int x)
{
  int y = 0x1234;
  int z = INT_MIN;
  return (x ^ y) - z;
}

unsigned int ftest12u(unsigned int x)
{
  unsigned int y = 0x1234;
  unsigned int z = (unsigned int)INT_MIN;
  return (x ^ y) - z;
}


void ftest(int a, int b)
{
  if (ftest1(a) != b)
    ASSERT (0);
  if (ftest2(a) != b)
    ASSERT (0);
  if (ftest3(a) != b)
    ASSERT (0);
  if (ftest4(a) != b)
    ASSERT (0);
  if (ftest5(a) != b)
    ASSERT (0);
  if (ftest6(a) != b)
    ASSERT (0);
  if (ftest7(a) != b)
    ASSERT (0);
  if (ftest8(a) != b)
    ASSERT (0);
  if (ftest9(a) != b)
    ASSERT (0);
  if (ftest10(a) != b)
    ASSERT (0);
  if (ftest11(a) != b)
    ASSERT (0);
  if (ftest12(a) != b)
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
  if (ftest7u(a) != b)
    ASSERT (0);
  if (ftest8u(a) != b)
    ASSERT (0);
  if (ftest9u(a) != b)
    ASSERT (0);
  if (ftest10u(a) != b)
    ASSERT (0);
  if (ftest11u(a) != b)
    ASSERT (0);
  if (ftest12u(a) != b)
    ASSERT (0);
}


void
testTortureExecute (void)
{
#if INT_MAX == 2147483647
  ftest(0x00000000,0x80001234);
  ftest(0x00001234,0x80000000);
  ftest(0x80000000,0x00001234);
  ftest(0x80001234,0x00000000);
  ftest(0x7fffffff,0xffffedcb);
  ftest(0xffffffff,0x7fffedcb);

  ftestu(0x00000000,0x80001234);
  ftestu(0x00001234,0x80000000);
  ftestu(0x80000000,0x00001234);
  ftestu(0x80001234,0x00000000);
  ftestu(0x7fffffff,0xffffedcb);
  ftestu(0xffffffff,0x7fffedcb);
#endif

#if INT_MAX == 32767
  ftest(0x0000,0x9234);
  ftest(0x1234,0x8000);
  ftest(0x8000,0x1234);
  ftest(0x9234,0x0000);
  ftest(0x7fff,0xedcb);
  ftest(0xffff,0x6dcb);

  ftestu(0x0000,0x9234);
  ftestu(0x8000,0x1234);
  ftestu(0x1234,0x8000);
  ftestu(0x9234,0x0000);
  ftestu(0x7fff,0xedcb);
  ftestu(0xffff,0x6dcb);
#endif

  return;
}

