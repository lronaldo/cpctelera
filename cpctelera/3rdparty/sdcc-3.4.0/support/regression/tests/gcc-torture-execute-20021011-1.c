/*
   20021011-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

/* PR opt/8165.  */

char buf[64];

void
testTortureExecute (void)
{
  int i;

  strcpy (buf, "mystring");
  if (strcmp (buf, "mystring") != 0)
    ASSERT (0);

  for (i = 0; i < 16; ++i)
    {
      strcpy (buf + i, "mystring");
      if (strcmp (buf + i, "mystring") != 0)
	ASSERT (0);
    }

  return;
}

