/*
   20000412-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

short int i = -1;
const char * const wordlist[207];

const char * const *
foo(void)
{
  register const char * const *wordptr = &wordlist[207u + i];
  return wordptr;
}

void
testTortureExecute (void)
{
  ASSERT (foo() == &wordlist[206]);
#if 0
  ASSERT (wordlist[0] == 0);
#endif
  return;
}
