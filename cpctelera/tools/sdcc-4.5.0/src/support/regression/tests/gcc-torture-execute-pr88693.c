/*
   pr88693.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#include <string.h>

/* PR tree-optimization/88693 */

void
foo (char *p)
{
  if (strlen (p) != 9)
    ASSERT (0);
}

void
quux (char *p)
{
  int i;
  for (i = 0; i < 100; i++)
    if (p[i] != 'x')
      ASSERT (0);
}

void
qux (void)
{
#if !(defined(__SDCC_pdk14) || defined(__SDCC_pdk15) && !defined(__SDCC_STACK_AUTO) || defined(__SDCC_mcs51)) // Lack of memory
  char b[100];
  memset (b, 'x', sizeof (b));
  quux (b);
#endif
}

void
bar (void)
{
#if !(defined(__SDCC_pdk14) || defined(__SDCC_pdk15) && !defined(__SDCC_STACK_AUTO) || defined(__SDCC_mcs51)) // Lack of memory
  static unsigned char u[9] = "abcdefghi";
  char b[100];
  memcpy (b, u, sizeof (u));
  b[sizeof (u)] = 0;
  foo (b);
#endif
}

void
baz (void)
{
#if !(defined(__SDCC_pdk14) || defined(__SDCC_pdk15) && !defined(__SDCC_STACK_AUTO) || defined(__SDCC_mcs51)) // Lack of memory
  static unsigned char u[] = { 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r' };
  char b[100];
  memcpy (b, u, sizeof (u));
  b[sizeof (u)] = 0;
  foo (b);
#endif
}

void
testTortureExecute (void)
{
  qux ();
  bar ();
  baz ();
  return;
}
