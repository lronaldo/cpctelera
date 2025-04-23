/*
   20030221-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

/* PR optimization/8613 */
/* Contributed by Glen Nakamura */
 
void
testTortureExecute (void)
{
  char buf[16] = "1234567890";
  char *p = buf;

  *p++ = (char) strlen (buf);

  if ((buf[0] != 10) || (p - buf != 1))
    ASSERT (0);

  return;
}

