/*
   bug-3025.c. (exclusive) or on a bit-field triggered an assertion in AST processing.
 */

#include <testfwk.h>

struct
{
    int x : 1;
} c;

void f()
{
    c.x ^ -1;
}

void g()
{
    c.x | -1;
}

void
testBug (void)
{
  f();
  g();
  return;
}

