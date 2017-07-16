/*
   bug-1929.c 
 */

#include <testfwk.h>

#define MASK 0x0f

unsigned char reg0;
unsigned char reg1;
signed char reg2;
signed char reg3;

void tAnd (void)
{
  reg0 &= ~MASK;
  ASSERT (reg0 == 0xa0);

  reg1 &= MASK;
  ASSERT (reg1 == 0x0a);

  reg2 &= ~MASK;
  ASSERT ((unsigned char) reg2 == 0xa0);

  reg3 &= MASK;
  ASSERT (reg3 == 0x0a);
}

void tOr (void)
{
  reg0 |= ~MASK;
  ASSERT (reg0 == 0xfa);

  reg1 |= MASK;
  ASSERT (reg1 == 0xaf);

  reg2 |= ~MASK;
  ASSERT ((unsigned char) reg2 == 0xfa);

  reg3 |= MASK;
  ASSERT ((unsigned char) reg3 == 0xaf);
}

void tXor (void)
{
  reg0 ^= ~MASK;
  ASSERT (reg0 == 0x5a);

  reg1 ^= MASK;
  ASSERT (reg1 == 0xa5);

  reg2 ^= ~MASK;
  ASSERT (reg2 == 0x5a);

  reg3 ^= MASK;
  ASSERT ((unsigned char) reg3 == 0xa5);
}

void testBug (void)
{
  reg0 = reg1 = reg2 = reg3 = 0xaa;
  tAnd();

  reg0 = reg1 = reg2 = reg3 = 0xaa;
  tOr();

  reg0 = reg1 = reg2 = reg3 = 0xaa;
  tXor();
}

