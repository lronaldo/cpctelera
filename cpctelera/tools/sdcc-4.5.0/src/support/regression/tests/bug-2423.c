/*
  bug-2423.c
 */

#include <testfwk.h>

#pragma disable_warning 147

struct foo_t {
  unsigned int a, b, c;
};

struct foo_t foo = {0xaaaa, 0x5555, 0xcccc, 0x3333, 0xffff, 0x0000, 0x1111, 0x2222, 0x4444, 0x6666};

void testBug (void)
{
  ASSERT (foo.a == 0xaaaa);
  ASSERT (foo.b == 0x5555);
  ASSERT (foo.c == 0xcccc);
}
