/*
   bug-3235.c. A function call was incorrectly considered to be recursive in the peephole optimizer when the name of the callee was a prefix of the caller's name.
   Required some further conditions, such as register arguments to the callee and tail-call optimization to trigger.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
func (int a) __z88dk_callee;

void
func_ (void)
{
  func (23);
  return;
}

int i;

void
testBug (void)
{
  func_();
  ASSERT (i == 23);
  return;
}

void
func (int a) __z88dk_callee
{
  i = a;
}

