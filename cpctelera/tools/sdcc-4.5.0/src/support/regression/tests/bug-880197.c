/*
    bug 880197
*/

#include <testfwk.h>

// no need to call this, it generates compiler error:
//   error: FATAL Compiler Internal Error in file 'gen.c'
//   line number '8381' : genPointerSet: illegal pointer type
//   Contact Author with source code

void
writeflash (__code unsigned char *cp, unsigned char val)
{
  *(__xdata unsigned char *)cp = val;
}

void
testBug (void)
{
}
