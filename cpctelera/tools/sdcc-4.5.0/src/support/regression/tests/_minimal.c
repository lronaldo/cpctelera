/** _minimal.c - a minimal regression test, picked from the MP8 fork of sdcc.
    The _ in the name was chosen to make this the first regression test,
    since it is meant to bring up regression testing for a new port,
    as it allows making the regression test framework work first.
*/
#include <testfwk.h>

void
testMinimal(void)
{
  ASSERT(1);
}

