/*
   pr61637.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR rtl-optimization/61673 */

char e;

void
bar (char x)
{
  if (x != 0x54 && x != (char) 0x87)
    ASSERT (0);
}

void
foo (const char *x)
{
  char d = x[0];
  int c = d;
  if ((c >= 0 && c <= 0x7f) == 0)
    e = d;
  bar (d);
}

void
baz (const char *x)
{
  char d = x[0];
  int c = d;
  if ((c >= 0 && c <= 0x7f) == 0)
    e = d;
}

void
testTortureExecute (void)
{
  const char c[] = { 0x54, 0x87 };
  e = 0x21;
  foo (c);
  if (e != 0x21)
    ASSERT (0);
  foo (c + 1);
  if (e != (char) 0x87)
    ASSERT (0);
  e = 0x21;
  baz (c);
  if (e != 0x21)
    ASSERT (0);
  baz (c + 1);
  if (e != (char) 0x87)
    ASSERT (0);
}
