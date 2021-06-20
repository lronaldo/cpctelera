/*
   while.c
*/

#include <testfwk.h>

char c1 = 0, c2 = 1;

void
testEmptyWhile(void)
{
  /* loops forever if bug ist present */
  do {} while (c1 && c2);

  /* other cases: */
  do {} while ( c1 &&  c1);
  do {} while ( c1 && !c2);
  do {} while (!c1 && !c2);
  do {} while ( c2 &&  c1);
  do {} while (!c2 &&  c1);
  do {} while (!c2 && !c1);
  do {} while (!c2 && !c2);

  do {} while ( c1 ||  c1);
  do {} while ( c1 || !c2);
  do {} while (!c2 ||  c1);
  do {} while (!c2 || !c2);

  ASSERT(1);
}
