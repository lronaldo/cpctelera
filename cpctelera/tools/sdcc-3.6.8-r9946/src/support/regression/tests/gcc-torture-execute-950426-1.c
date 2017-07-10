/*
   950426-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#pragma disable_warning 196
#endif

#include <string.h>

struct tag {
  int m1;
  char *m2[5];
} s1, *p1;

int i;

int func1(int *p);

void foo (char *s);

void
testTortureExecute (void)
{
  s1.m1 = -1;
  p1 = &s1;

  if ( func1( &p1->m1 ) == -1 )
    foo ("ok");
  else
    ASSERT (0);

  i = 3;
  s1.m2[3]= "123";

  if ( strlen( (p1->m2[i])++ ) == 3 )
    foo ("ok");
  else
    ASSERT (0);

  return;
}

int func1(int *p) { return(*p); }

void foo (char *s) {}

