/*
bf-layout-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#if 0 // Enable when SDCC supports bit-fields wider than 16 bits
struct { long f8:8; long f24:24; } a;
struct { long f32:32; } b;
#endif

void
testTortureExecute (void)
{
#if 0
  if (sizeof (a) != sizeof (b))
    ASSERT (0);
  return;
#endif
}
