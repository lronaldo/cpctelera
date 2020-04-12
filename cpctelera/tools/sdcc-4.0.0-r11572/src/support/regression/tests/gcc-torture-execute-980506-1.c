/*
   980506-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct decision
{
  char enforce_mode;             
  struct decision *next;         
};


static void
clear_modes (register struct decision *p)
{
  goto blah;

foo:
  p->enforce_mode = 0;
blah:
  if (p)
    goto foo;
}

void
testTortureExecute (void)
{
  struct decision *p = 0;
  clear_modes (p);
  return;
}

