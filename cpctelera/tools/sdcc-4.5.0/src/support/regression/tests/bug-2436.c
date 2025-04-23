/*
  bug-2436.c
 */

#include <testfwk.h>

char *q (char *q0, char *q1)
{
  return q0[0] > q1[0] ? q0 : q1; 
}

void *b (char *b0, char *b1, char *b2)
{
  char *a = b0[0] > b1[0] ? b0 : b1;
  return a[0] > b2[0] ? a : b2;
}

char a0[2] = {'6', 0};
char a1[2] = {'3', 0};
char a2[2] = {'9', 0};
char a3[2] = {'0', 0};

char *foo (void)
{
  return q(a0, b(a1, a2, a3));
}

void testBug (void)
{
  ASSERT (foo () == a2);
}
