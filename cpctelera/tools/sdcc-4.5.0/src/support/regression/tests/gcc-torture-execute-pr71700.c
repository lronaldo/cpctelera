/*
   pr71700.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct S
{
  signed f0 : 16;
  unsigned f1 : 1;
};

int b;
static struct S c[] = {{-1, 0}, {-1, 0}};
struct S d;

void
testTortureExecute (void)
{
#if 0 // TODO: Enable when SDCC supports struct initalization by assignment!
  struct S e = c[0];
  d = e;
  if (d.f1 != 0)
    ASSERT (0);
  return;
#endif
}
