/*
   20050125-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Verify that the CALL side effect isn't optimized away.  */
/* Contributed by Greg Parker  25 Jan 2005  <gparker@apple.com> */

#include <stdlib.h>
#include <stdio.h>

struct parse {
  char *next;
  char *end;
  int error;
};

int seterr(struct parse *p, int err)
{
  p->error = err;
  return 0;
}

void bracket_empty(struct parse *p)
{
  if (((p->next < p->end) && (*p->next++) == ']')  ||  seterr(p, 7)) { }
}

void testTortureExecute(void)
{
  struct parse p;
  char x;
  p.next = p.end = &x;

  p.error = 0;
  bracket_empty(&p);
  if (p.error != 7)
    ASSERT (0);

  return;
}
