/*
   920202-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

static int rule_text_needs_stack_pop = 0;
static int input_stack_pos = 1;

f (void)
{
  rule_text_needs_stack_pop = 1;

  if (input_stack_pos <= 0)
    return 1;
  else
    return 0;
}

void
testTortureExecute (void)
{
  f ();
  return;
}

