/*
   patch-466.c - tests for some peephole optimizer rules from patch #466
   to optimize theuse of MCs-51 bit instructions.
 */
 
#include <testfwk.h>

#include <stdint.h>
#include <stdbool.h>

uint8_t aaa;
uint8_t bbb;

uint8_t get_bleh (uint8_t x)
{
  bool x0 = x != aaa;
  bool x1 = x != bbb;

  return (!x0 | !x1) ? 0xFF : 0x00;
}

void testPatch(void)
{
  aaa = 0;
  bbb = 0;
  ASSERT (get_bleh (0) == 0xff);
  ASSERT (get_bleh (1) == 0x00);

  aaa = 0;
  bbb = 1;
  ASSERT (get_bleh (0) == 0xff);
  ASSERT (get_bleh (1) == 0xff);

  aaa = 1;
  bbb = 0;
  ASSERT (get_bleh (0) == 0xff);
  ASSERT (get_bleh (1) == 0xff);

  aaa = 1;
  bbb = 1;
  ASSERT (get_bleh (0) == 0x00);
  ASSERT (get_bleh (1) == 0xff);
}

