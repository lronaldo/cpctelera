/*
   bug-2381.c
 */

#include <testfwk.h>

#pragma disable_warning 88

#define VAL 0x5555

void foo(char *p)
{
  ASSERT (p == (char *) VAL);
}

void testBug(void)
{
  foo (&(*((char *) VAL)));
  foo (&(*(&(*((char *) VAL)))));
}
