/** bug-2102.c
*/
#include <testfwk.h>
#include <stdlib.h>

#pragma disable_warning 219

struct tst {
  int a, b;
  int p[0];
};

void
testBug(void)
{
  struct tst w[2] = {{0x5555, 0x3333}, {0x1111, 0x2222}}; // will rise warning 219
  struct tst *p = w; // won't rise warning 219
  ASSERT (w[1].a == 0x1111);
  ASSERT (w[1].b == 0x2222);
  p->p[0] = 0x4444;
  ASSERT (w[1].a == 0x4444);
  ASSERT (w[1].b == 0x2222);
}
