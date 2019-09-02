/*
   20041212-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) //Bug #2903
/* A function pointer compared with a void pointer should not be canonicalized.
   See PR middle-end/17564.  */
void *f (void);
void *
f (void)
{
  return f;
}
#endif // There was no line here.
void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) //Bug #2903
  if (f () != f)
    ASSERT (0);
  return;
#endif
}
