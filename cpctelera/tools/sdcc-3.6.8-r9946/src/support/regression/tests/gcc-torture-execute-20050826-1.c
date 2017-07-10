/*
   20050826-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

#if defined __SDCC_MODEL_SMALL || defined __SDCC_MODEL_MEDIUM
#define SIZE 64     // the available memory is limited
#else
#define SIZE 2048
#endif

/* PR rtl-optimization/23561 */

struct A
{
  char a1[1];
  char a2[5];
  char a3[1];
  char a4[SIZE - 7];
} a;

void
bar (struct A *x)
{
  size_t i;
  ASSERTFALSE (memcmp (x, "\1HELLO\1", sizeof "\1HELLO\1"));
  for (i = 0; i < sizeof (x->a4); i++)
    ASSERTFALSE (x->a4[i]);
}

int
foo (void)
{
  memset (&a, 0, sizeof (a));
  a.a1[0] = 1;
  memcpy (a.a2, "HELLO", sizeof "HELLO");
  a.a3[0] = 1;
  bar (&a);
  return 0;
}

void
testTortureExecute (void)
{
  foo ();
}

