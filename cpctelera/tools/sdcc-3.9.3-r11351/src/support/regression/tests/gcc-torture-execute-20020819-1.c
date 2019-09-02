/*
   20020819-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

foo ()
{
  return 0;
}

void
testTortureExecute (void)
{
  int i, j, k, ccp_bad = 0;

  for (i = 0; i < 10; i++)
    {
      for (j = 0; j < 10; j++)
	if (foo ())
	  ccp_bad = 1;
    
      k = ccp_bad != 0;
      if (k)
	ASSERT (0);
    }

  return;
}
