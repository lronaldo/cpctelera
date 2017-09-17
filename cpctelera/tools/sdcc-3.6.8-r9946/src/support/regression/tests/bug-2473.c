/*
   bug-2473.c
 */

#include <testfwk.h>

struct foo
{
  unsigned char *base;
};

unsigned char s[4] = {'a', 'b', 'c', 0};
struct foo bar = {s};

int ugetc (const void *p)
{
  return *((const char *) p);
}

void netat_outbyte (unsigned char v)
{
  ASSERT (v == 'a');
}

void testBug (void)
{
  netat_outbyte (ugetc (bar.base++));
  ASSERT (*bar.base == 'b');
}

