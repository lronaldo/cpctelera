/*
   20040218-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR target/14209.  Bug in cris.md, shrinking access size of
   postincrement.
   Origin: <hp@axis.com>.  */

long int xb (long int *y);
long int xw (long int *y);
short int yb (short int *y);

long int xb (long int *y)
{
  long int xx = *y & 255;
  return xx + y[1];
}

long int xw (long int *y)
{
  long int xx = *y & 65535;
  return xx + y[1];
}

short int yb (short int *y)
{
  short int xx = *y & 255;
  return xx + y[1];
}

void testTortureExecute (void)
{
  long int y[] = {-1, 16000};
  short int yw[] = {-1, 16000};

  if (xb (y) != 16255
      || xw (y) != 81535
      || yb (yw) != 16255)
    ASSERT (0);
  return;
}
