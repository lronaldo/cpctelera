/* promoting bit to char */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_sdcc99
#endif

#include <stdbool.h>

#if !defined(__bool_true_false_are_defined)
volatile int a = 1, b = 1;
#else
volatile bool a = 1, b = 1;
#endif

char
foo (void)
{
  return (a << 1) | b;
}

static void
testBitToCharPromotion(void)
{
  ASSERT(foo() == 3);
}
