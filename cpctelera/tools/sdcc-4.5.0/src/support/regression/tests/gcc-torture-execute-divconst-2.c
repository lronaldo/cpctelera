/*
   divconst-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#pragma disable_warning 85

#ifndef __SDCC_pdk14 // Lack of memory
long
f (long x)
{
  return x / (-0x7fffffffL - 1L);
}

long
r (long x)
{
  return x % (-0x7fffffffL - 1L);
}

#if !(defined(__SDCC_pic14) && !defined(__SDCC_PIC14_ENHANCED)) // Pseudo-stack size limit
/* Since we have a negative divisor, this equation must hold for the
   results of / and %; no specific results are guaranteed.  */
long
std_eqn (long num, long denom, long quot, long rem)
{
  /* For completeness, a check for "ABS (rem) < ABS (denom)" belongs here,
     but causes trouble on 32-bit machines and isn't worthwhile.  */
  return quot * (-0x7fffffffL - 1L) + rem == num;
}
#endif

long nums[] =
{
  -1L, 0x7fffffffL, -0x7fffffffL - 1L
};
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
  int i;

  for (i = 0;
       i < sizeof (nums) / sizeof (nums[0]);
       i++)
#if !(defined(__SDCC_pic14) && !defined(__SDCC_PIC14_ENHANCED)) // Pseudo-stack size limit
    if (std_eqn (nums[i], -0x7fffffffL - 1L, f (nums[i]), r (nums[i])) == 0)
      ASSERT (0);
#endif
#endif
#endif
  return;
}
