/*
   20000121-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

// Some ports do not support long long yet
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
void big(long long u) { }

void doit(unsigned int a,unsigned int b,char *id)
{
  big(*id);
  big(a);
  big(b);
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  doit(1,1,"\n");
  return;
#endif
}

