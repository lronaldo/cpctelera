/*
   bug2944399.c
 */

#include <testfwk.h>

unsigned short Addr1(void)
{
  return (0x1234);
 }

unsigned short func16(unsigned short Dummy)
{
  return Dummy;
}

void testBug1(void)
{
  unsigned short a;

  a = 0x0101 | Addr1();
  ASSERT (func16(a) == (0x0101 | 0x1234));
}

void testBug2(void)
{
  unsigned short a;

  a = 0x0101 & Addr1();
  ASSERT (func16(a) == (0x0101 & 0x1234));
}

void testBug3(void)
{
  unsigned short a;

  a = 0x0101 ^ Addr1();
  ASSERT (func16(a) == (0x0101 ^ 0x1234));
}

void testBug4(void)
{
  unsigned short a;

  a = Addr1();
  a |= 0x0101;
  ASSERT (func16(a) == (0x0101 | 0x1234));
}
