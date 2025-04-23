/*  Bug 1253, was bug 1609244
 */

#include <testfwk.h>

typedef char * PCHAR;

char KAR;

// bug.c:7: warning 60: function return value mismatch from type 'char near* '
// to type 'char generic* unknown type'
// bug.c:5: warning 59: function 'foo' must return value

PCHAR foo(void) __reentrant //this fails
{
  return &KAR;
}

PCHAR bar(void) // this is ok
{
  return &KAR;
}

void
testBug(void)
{
  ASSERT(foo() == bar());
}

