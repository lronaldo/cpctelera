/*
   20020328-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int b = 0;

void
func (void) { }

void
tryit(int x)
{
  ASSERT (x == 20);
}

void
testTortureExecute (void)
{
  int a = 0;

  if (b)
    func();

  /* simplify_and_const_int would incorrectly omit the mask in
     the line below.  */
  tryit ((a + 23) & 0xfffffffc);
  return;
}
