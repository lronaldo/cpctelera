/*
   990531-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

   unsigned long bad(int reg, unsigned long inWord)
   {
       union {
           unsigned long word;
           unsigned char byte[4];
       } data;

       data.word = inWord;
       data.byte[reg] = 0;

       return data.word;
   }

void
testTortureExecute (void)
{
  /* XXX This test could be generalized.  */
  if (sizeof (long) != 4)
    return;

  if (bad (0, 0xdeadbeef) == 0xdeadbeef)
    ASSERT (0);
  return;
}

