/*
   20000603-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

/* It is not clear whether this test is conforming.  See DR#236
   http://www.open-std.org/jtc1/sc22/wg14/www/docs/dr_236.htm.  However,
   there seems to be consensus that the presence of a union to aggregate
   struct s1 and struct s2 should make it conforming.  */
struct s1 { double d; };
struct s2 { double d; };
union u { struct s1 x; struct s2 y; };

#ifndef __SDCC_pdk14 // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
double f(struct s1 *a, struct s2 *b)
{
  a->d = 1.0;
  return b->d + 1.0;
}
#endif
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
  union u a;
  a.x.d = 0.0;
  if (f (&a.x, &a.y) != 2.0)
    ASSERT (0);
  return;
#endif
#endif
}

