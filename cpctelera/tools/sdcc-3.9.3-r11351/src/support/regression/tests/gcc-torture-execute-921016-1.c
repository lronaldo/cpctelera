/*
   921016-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pic16 // TODO: enable when the pic16 ports supports bitfields of size greater than 8 bits!
  int j=1081;
  struct
    {
      signed int m:11;
   } l;
  if((l.m = j) == j)
    ASSERT(0);
  return;
#endif
}

