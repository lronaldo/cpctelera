/*
   20070724-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

static unsigned char magic[] = "\235";
static unsigned char value = '\235';

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && __GNUC__ < 5)
  if (value != magic[0])
    ASSERT (0);
  return;
#endif
}
