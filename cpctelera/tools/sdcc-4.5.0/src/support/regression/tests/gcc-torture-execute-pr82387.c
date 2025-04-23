/*
pr82387.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* PR tree-optimization/82387 */

#if 0 // TODO: Enable when SDCC accepts the struct initializatio below
struct A { int b; };
int f = 1;

struct A
foo (void)
{
  struct A h[] = { 
    {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, 
    {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, 
    {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, 
    {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, 
    {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, 
    {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, 
    {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, {1}, 
  };
  return h[24];
}
#endif
void
testTortureExecute (void)
{
#if 0
  struct A i = foo (), j = i;
  j.b && (f = 0);
  return f;
#endif
}
