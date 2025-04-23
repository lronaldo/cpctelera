/*
    bug-2973.c
*/

#include <testfwk.h>

typedef int *foo_t;

void f(const foo_t x)
{
  *x = 1;
}

// nothing to run - bug is a compilation failure

void testBug (void)
{
}

