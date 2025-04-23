/*
    bug 1921073
*/

#include <testfwk.h>

void
f1 (char c, const void * p)
{
  unsigned long v = (unsigned long)p;
  c;
  v;
#if defined(__SDCC_mcs51)
  ASSERT ((unsigned char)(v >> 16) == 0x80);
#endif
}

void
f2 (const void * p)
{
  unsigned long v = (unsigned long)p;
  v;
#if defined(__SDCC_mcs51)
  ASSERT ((unsigned char)(v>>16)==0x80);
#endif
}

void
testBug (void)
{
  f1(5, (__code void *)0x1234);
  f2((__code void *)0x1234);
}

