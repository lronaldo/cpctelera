/*
   961017-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testTortureExecute (void)
{
  int i = 0;


  if (sizeof (unsigned long int) == 4)
    {
      unsigned long int z = 0;

      do {
	z -= 0x000040000;
	i++;
	if (i > 0x0004000)
	  ASSERT (0);
      } while (z > 0);
      return;
    }
  else if (sizeof (unsigned int) == 4)
    {
      unsigned int z = 0;

      do {
	z -= 0x000040000;
	i++;
	if (i > 0x0004000)
	  ASSERT (0);
      } while (z > 0);
      return;
    }
  else
    return;
}
