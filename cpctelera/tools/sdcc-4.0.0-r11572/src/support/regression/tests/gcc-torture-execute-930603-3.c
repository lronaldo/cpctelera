/*
   930603-3.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int f (unsigned char *b, int c)
{
  unsigned long v = 0;
  switch (c)
    {
    case 'd':
      v = ((unsigned long)b[0] << 8) + b[1];
      v >>= 9;
      break;

    case 'k':
      v = b[3] >> 4;
      break;

    default:
      ASSERT (0);
    }

  return v;
}
void
testTortureExecute (void)
{
  char buf[4];
  buf[0] = 170; buf[1] = 5;
  if (f (buf, 'd') != 85)
    ASSERT (0);
  return;
}

