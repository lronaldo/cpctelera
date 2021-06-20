/*
   bug-2197.c - originally part of the tests from the execute part of the gcc torture suite (see 20020503-1.c).
   Heavily modified to reproduce the underlying issue of bug 2197 on multiple architectures.
 */

#include <testfwk.h>

#if defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM) || \
    (defined(__SDCC_mcs51) && defined(__SDCC_STACK_AUTO))
#define N	32
#else
#define N	128
#endif

/* PR 6534 */
/* GCSE unified the two i<0 tests, but if-conversion to ui=abs(i)
   inserted the code at the wrong place corrupting the i<0 test.  */

static char *
inttostr (long i, char buf[N])
{
  unsigned long ui = i;
  char *p = buf + (N-1);
  *p = '\0';
  if (i < 0)
    ui = -ui;
  do
    *--p = '0' + ui % 10;
  while ((ui /= 10) != 0);
  if (i < 0)
    *--p = '-';
  return p;
}

void
testTortureExecute (void)
{
  char buf[N], *p;

  p = inttostr (-1, buf);

  ASSERT(p[0] == '-');
  ASSERT(p[1] == '1');
  ASSERT(p[2] == '\0');
  ASSERT(p == buf + (N-1) - 2);

  return;
}
