/*
   960608-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

typedef struct
{
  unsigned char a  : 2;
  unsigned char b  : 3;
  unsigned char c  : 1;
  unsigned char d  : 1;
  unsigned char e  : 1;
} a_struct;

int foo (a_struct *flags)
{
  return (flags->c != 0
	  || flags->d != 1
	  || flags->e != 1
	  || flags->a != 2
	  || flags->b != 3);
}

void
testTortureExecute (void)
{
  a_struct flags;

  flags.c  = 0;
  flags.d  = 1;
  flags.e  = 1;
  flags.a  = 2;
  flags.b  = 3;

  if (foo (&flags) != 0)
    ASSERT (0);
  else
    return;
}

