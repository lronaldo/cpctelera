/*
   pr37125.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_pdk14) && !defined (__SDCC_pdk15) // Bug #2874
static inline unsigned int
mod_rhs(int rhs)
{
  if (rhs == 0) return 1;
  return rhs;
}

void func_44 (unsigned int p_45);
void func_44 (unsigned int p_45)
{
  if (!((p_45 * -9) % mod_rhs (-9))) {
      ASSERT (0);
  }
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) && !defined (__SDCC_pdk15) // Bug #2874
  func_44 (2);
  return;
#endif
}

