/*
    bug 1749275
*/

#include <testfwk.h>

__xdata char baz;

// no need to call this, it generates compiler error:
//   Internal error: validateOpType failed in
//   OP_SYM_TYPE(IC_LEFT(pl->ic)) @ peep.c:226:
//   expected symbol, got value
void
foo(void)
{
  baz |= 5;
  (*(void (*) ()) 0) ();
}

void
testBug (void)
{
  ASSERT (1);
}
