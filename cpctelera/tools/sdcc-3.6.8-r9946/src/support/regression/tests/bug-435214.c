/* bug-435214.c
 */
#include <testfwk.h>

unsigned long divide(long a)
{
    return a/512ul;
}

void
testDivide(void)
{
  ASSERT(divide(1300) == 2);
  ASSERT(divide(0x12345678) == 0x91A2B);
}

