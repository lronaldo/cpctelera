/*
   950929-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

int f (char *p) { }

void
testTortureExecute (void)
{
  char c;
  char c2;
  int i = 0;
  char *pc = &c;
  char *pc2 = &c2;
  int *pi = &i;

  *pc2 = 1;
  *pi = 1;
  *pc2 &= *pi;
  f (pc2);
  *pc2 = 1;
  *pc2 &= *pi;
  if (*pc2 != 1)
    ASSERT (0);
  return;
}

