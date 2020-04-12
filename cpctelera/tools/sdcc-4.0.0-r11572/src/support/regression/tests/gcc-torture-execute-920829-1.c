/*
   920829-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports long long in these ports!
#if !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
long long c=2863311530LL,c3=2863311530LL*3;
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) // Lack of memory.
  if(c*3!=c3)
    ASSERT(0);
  return;
#endif
}

