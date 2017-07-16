/*
   930513-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>
#include <stdio.h>

char buf[12];

void f (int (*fp)(char *, const char *, ...))
{
  (*fp)(buf, "%.0f", 5.0);
}

void
testTortureExecute (void)
{
  f (&sprintf);
  ASSERT ((buf[0] == '<' && buf[1] == 'N') ||   // "<NO FLOAT>""
          (buf[0] == '5' && buf[1] == 0));      // "5"
}
