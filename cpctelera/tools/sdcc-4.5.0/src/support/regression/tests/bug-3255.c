/*
   bug-3132.c
   A bug in the frontend in the handling of _Generic matches for pointers.
 */

#include <testfwk.h>

#include <stdint.h>

int *p;

void f(void)
{
    _Generic(p, int * : 1, long * : 2); // Internal error in SDCCast.c
}

void
testBug (void)
{
  f();
}

