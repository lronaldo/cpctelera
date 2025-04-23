/*
    bug-2973.c
*/

#include <testfwk.h>

typedef int *foo_t;

void f(foo_t *const x)
{
  *x = 0;
}

// nothing to run - bug is a compilation failure

void testBug (void)
{
}

