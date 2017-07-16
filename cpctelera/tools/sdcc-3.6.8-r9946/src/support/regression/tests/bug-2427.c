/*
  bug-2427.c
 */

#include <testfwk.h>
#include <string.h>

static char b0[10] = "1\0002\0003";
static char c0[10] = {'1', 0, '2', 0, '3', 0, 0, 0, 0, 0};

static char foo (char a)
{
  return a + 1;
}

static char b1[15] = "A\000B\000C";
static char c1[15] = {'A', 0, 'B', 0, 'C', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void testBug (void)
{
  ASSERT (memcmp (b0, c0, sizeof (c0)) == 0);
  ASSERT (memcmp (b1, c1, sizeof (c1)) == 0);
  ASSERT (foo ('a') == 'b');
}
