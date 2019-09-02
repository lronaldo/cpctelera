/*
   20020215-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Test failed on an architecture that:

   - had 16-bit registers,
   - passed 64-bit structures in registers,
   - only allowed SImode values in even numbered registers.

   Before reload, s.i2 in foo() was represented as:

	(subreg:SI (reg:DI 0) 2)

   find_dummy_reload would return (reg:SI 1) for the subreg reload,
   despite that not being a valid register.  */

#if 0 // TODO: Enable when struct can be passed and returned!
struct s
{
  short i1;
  long i2;
  short i3;
};

struct s foo (struct s s)
{
  s.i2++;
  return s;
}
#endif

void
testTortureExecute (void)
{
#if 0
  struct s s = foo ((struct s) { 1000, 2000L, 3000 });
  if (s.i1 != 1000 || s.i2 != 2001L || s.i3 != 3000)
    ASSERT (0);
#endif
}

