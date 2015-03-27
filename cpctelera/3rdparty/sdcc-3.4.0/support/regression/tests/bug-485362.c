/** Shows segfault.
    type: int
 */
#include <testfwk.h>

void
spoil({type} f)
{
  UNUSED(f);
}

void
testDivBySelf(void)
{
  volatile {type} left, result;

  left = 17;
  result = left/left;

  spoil(result);
}
