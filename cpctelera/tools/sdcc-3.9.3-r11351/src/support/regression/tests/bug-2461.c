/*
   bug-2461.c
*/

#include <testfwk.h>

char foo (const char *p)
{
  return p[0];
}

int i = 0;

void testBug (void)
{
  char c[] = "1234";
  ASSERT (foo (i ? (const char *) c : "test") == 't');
  i = 1;
  ASSERT (foo (i ? (const char *) c : "test") == '1');
}

