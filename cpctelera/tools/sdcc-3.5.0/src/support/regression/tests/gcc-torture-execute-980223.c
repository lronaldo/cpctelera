/*
   980223.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports struct!
#if 0
typedef struct { char *addr; long type; } object;

object bar (object blah)
{
  ASSERT(0);
}

object foo (object x, object y)
{
  object z = *(object*)(x.addr);
  if (z.type & 64)
    {
      y = *(object*)(z.addr+sizeof(object));
      z = *(object*)(z.addr);
      if (z.type & 64)
        y = bar(y);
    }
  return y;
}

int nil;
object cons1[2] = { {(char *) &nil, 0}, {(char *) &nil, 0} };
object cons2[2] = { {(char *) &cons1, 64}, {(char *) &nil, 0} };
#endif

void
testTortureExecute (void)
{
#if 0
  object x = {(char *) &cons2, 64};
  object y = {(char *) &nil, 0};
  object three = foo(x,y);
  return;
#endif
}

