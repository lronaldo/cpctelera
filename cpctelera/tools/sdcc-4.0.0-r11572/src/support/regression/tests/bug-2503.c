/*
    bug-2503
*/

#include <testfwk.h>

/* Make sure side-effects are preserved when optimizing */
/* conditional tests. */

unsigned int x;

unsigned int test1(void)
{
  unsigned int a=0;
  if ((a=0x55) >= 0)
    x++;
  return a;
}

unsigned int test2(void)
{
  return x++;
}

void
testBug(void)
{
  x = 0;
  ASSERT(test1() == 0x55);
  ASSERT(x == 1);
  ASSERT(test1() == test1());
  ASSERT(x == 3);
  ASSERT((test2() - test2()) != 0);
  ASSERT(x == 5);
}
