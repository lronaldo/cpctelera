/*
   20020216-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR c/3444
   This used to fail because bitwise xor was improperly computed in char type
   and sign extended to int type.  */

signed char c = (signed char) 0xffffffff;

int foo (void)
{
  return (unsigned short) c ^ (signed char) 0x99999999;
}

void testTortureExecute (void)
{
  if ((unsigned char) -1 != 0xff
      || sizeof (short) != 2
      || sizeof (int) != 4)
    return;
  if (foo () != (int) 0xffff0066)
    ASSERT (0);
  return;
}

