/*
   20190228-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

/* PR tree-optimization/89536 */
/* Testcase by Zhendong Su <su@cs.ucdavis.edu> */

int a = 1;

void
testTortureExecute (void)
{
  a = ~(a && 1); 
  if (a < -1)
    a = ~a;
  
  if (!a)
    ASSERT (0);

  return;
}
