/*
   20021219-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#pragma disable_warning 85

/* PR optimization/8988 */
/* Contributed by Kevin Easton */

void foo(char *p1, char **p2)
{}
 
void testTortureExecute(void)
{
  char str[] = "foo { xx }";
  char *ptr = str + 5;

  foo(ptr, &ptr);

  while (*ptr && (*ptr == 13 || *ptr == 32))
    ptr++;

  return;
}
