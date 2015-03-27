/* replace optimized parameter tree returned by decorateType */

#include <testfwk.h>

char
foo(char c)
{
  return c;
}

void
testReplaceParameterTree(void)
{
  ASSERT(foo (0 ? 1 : 2) == 2);
}
