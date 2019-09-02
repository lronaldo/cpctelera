/*
   20140425-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR target/60941 */
/* Reported by Martin Husemann <martin@netbsd.org> */

static void
set (unsigned long *l)
{
  *l = 31;
}

void
testTortureExecute (void)
{
  unsigned long l;
  int i;

  set (&l);
  i = (int) l;
  l = (unsigned long)(2U << i);
  if (l != 0)
    ASSERT (0);
}
