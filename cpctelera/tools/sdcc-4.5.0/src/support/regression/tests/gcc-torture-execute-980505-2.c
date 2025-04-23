/*
   980505-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

typedef unsigned short Uint16;
typedef unsigned int Uint;

Uint f ()
{
        Uint16 token;
        Uint count;
        static Uint16 values[1] = {0x9300};

        token = values[0];
        count = token >> 8;

        return count;
}

void
testTortureExecute (void)
{
  if (f () != 0x93)
    ASSERT (0);
  return;
}

