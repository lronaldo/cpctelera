/*
pr82388.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* PR tree-optimization/82388 */

#if 0 // TODO: Enable when SDCC can initialize with extra braces
struct A { int b; int c; int d; } e;

struct A
foo (void)
{
  struct A h[30] = {{0,0,0}};
  return h[29]; 
}
#endif
void
testTortureExecute (void)
{
#if 0
  e = foo ();
  return e.b; 
#endif
}
