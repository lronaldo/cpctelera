/* bug-1376320.c

   copy signedness while replacing operands
 */
#include <testfwk.h>

static void
testSign(void)
{
  signed long l;

  l = 3;
  l -= 5ul;
  ASSERT(l < 0);
}
