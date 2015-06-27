/* bug-1805702.c
 */

#include <testfwk.h>

int foo;
extern int foo;

void
test(void)
{
  foo = 10;

  ASSERT(foo == 10);
}

/* compile time check for compiler defined functions (cdef) */

float __fsmul (float, float);

float __fsmul (float a1, float a2) {
  /* just for tesing... */
  return (a1 + a2);
}
