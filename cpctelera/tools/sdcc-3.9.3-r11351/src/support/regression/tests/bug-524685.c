/* Division by powers of two.
 */
#include <testfwk.h>

void
testDivPow2(void)
{
  volatile int left;

  left = -18;
  ASSERT(left/4 == (-18/4));
}
