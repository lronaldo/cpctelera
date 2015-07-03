/*
   bug2862308.c
*/

#include <testfwk.h>

char case1(void)
{
  char j = 0;
  char *z = &j;

  j = 1;

  if (j)
    return 1;
  else
    return 0; /* We incorrectly end up here when using '*z' instead of '*(char *) &j'. */
}

char case2(void)
{
  char j = 0;
  char *z = &j;

  j = 1;

  if (*(char *) &j) /* Does *not* work with:
                     * sdcc -mmcs51 --model-large sample.c */
    return 1;
  else
    return 0; /* We incorrectly end up here when using '*z' instead of '*(char *) &j'. */
}

char case3(void)
{
  char j = 0;
  char *z = &j;

  j = 1;

  if (*z) /* Does *not* work with:
           * sdcc -mmcs51 --model-large sample.c
           * sdcc -mmcs51 sample.c */
    return 1;
  else
    return 0; /* We incorrectly end up here when using '*z' instead of '*(char *) &j'. */
}

void testBug(void)
{
  ASSERT(case1());
  ASSERT(case2());
  ASSERT(case3());
}
