/*
    bug-2384.c
*/

#include <testfwk.h>

typedef char chararr[4];

const char a[4] = {'1', '2', '3', '4'};
const chararr b = {'5', '6', '7', '8'};
const char c[4] = {'a', 'b', 'c', 'd'};

void testBug (void)
{
#if defined (__SDCC)
  const char *pa = a, *pb = b, *pc = c;
  ASSERT (pa[4] == '5');
  ASSERT (pb[-1] == '4');
  ASSERT (pb[4] == 'a');
  ASSERT (pc[-1] == '8');
#endif
}
