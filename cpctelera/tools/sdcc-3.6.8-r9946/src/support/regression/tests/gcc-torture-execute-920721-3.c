/*
   920721-3.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

static inline int fu (unsigned short data)
{
  return data;
}

void ru(int i)
{
   if(fu(i++)!=5)ASSERT(0);
   if(fu(++i)!=7)ASSERT(0);
}

static inline int fs (signed short data)
{
  return data;
}

void rs(int i)
{
   if(fs(i++)!=5)ASSERT(0);
   if(fs(++i)!=7)ASSERT(0);
}


void
testTortureExecute (void)
{
  ru(5);
  rs(5);
  return;
}

