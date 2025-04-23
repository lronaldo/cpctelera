/*
restrict-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/*  PR rtl-optimization/16536
    Origin:  Jeremy Denise      <jeremy.denise@libertysurf.fr>
    Reduced: Wolfgang Bangerth  <bangerth@dealii.org>
             Volker Reichelt    <reichelt@igpm.rwth-aachen.de>  */
/* { dg-options "-fgnu89-inline" } */

typedef struct
{
  int i, dummy;
} A;
#if 0 // TODO: Enable when compound literals are supported
inline A foo (const A* p, const A* q)
{
  return (A){p->i+q->i};
}

void bar (A* __restrict__ p)
{
  *p=foo(p,p);
  if (p->i!=2)
    abort();
}
#endif
void
testTortureExecute (void)
{
#if 0 // TODO: Enable when compound literals are supported
  A a={1};
  bar(&a);
  return 0;
#endif
}
