/*
   pr66757.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

/* PR tree-optimization/66757 */
/* Testcase by Zhendong Su <su@cs.ucdavis.edu> */

int a, b;

void
testTortureExecute (void)
{
  unsigned int t = (unsigned char) (~b); 

  if ((t ^ 1) / 255)
    ASSERT (0); 

  return;
}
