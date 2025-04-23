/*
pr81913.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#include <stdint.h>

/* PR tree-optimization/81913 */

typedef uint8_t u8;
typedef uint32_t u32;

static u32
b (u8 d, u32 e, u32 g)
{
  do
    {
      e += g + 1;
      d--;
    }
  while (d >= (u8) e);

  return e;
}

void
testTortureExecute (void)
{
  u32 x = b (1, -0x378704, ~0xba64fc);
  if (x != 0xd93190d0)
    ASSERT (0);
  return;
}

