/*
   pr19606.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined (__SDCC_pic14) && !defined(__SDCC_pdk14) // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
/* PR c/19606
   The C front end used to shorten the type of a division to a type
   that does not preserve the semantics of the original computation.
   Make sure that won't happen.  */

signed char a = -4;

int
foo (void)
{
  return ((unsigned int) (signed int) a) / 2LL;
}

int
bar (void)
{
  return ((unsigned int) (signed int) a) % 5LL;
}
#endif
#endif

void
testTortureExecute (void)
{
#if !defined (__SDCC_pic14) && !defined(__SDCC_pdk14) // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
#if !defined (PORT_HOST) // failed in test-host
  int r;

  r = foo ();
  if (r != ((unsigned int) (signed int) (signed char) -4) / 2LL)
    ASSERT (0);

  r = bar ();
  if (r != ((unsigned int) (signed int) (signed char) -4) % 5LL)
    ASSERT (0);

  return;
#endif
#endif
#endif
}
