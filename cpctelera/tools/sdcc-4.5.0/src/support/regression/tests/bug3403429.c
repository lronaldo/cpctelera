/*
   bug3403429.c
*/

#include "testfwk.h"

void PTBL (signed char col, signed char row, unsigned char spN)
{
  col;
  row;
  spN;
}

signed char C [4+1][2+1];
unsigned int gm;

signed char i;

void Sub_9000 (void)
{
  {
    unsigned char i, j;
    for( i=0; i<=36; i+=4 )
    {
      for( j=0; j<=23; j+=8 )
      {
        PTBL(i,j,98);
        PTBL(30-i,20-j,98);
      }
    }
  }

  if( gm==0 )
  {
    C[ 1][1]=10; C[ 1][2]=10;
    C[ 2][1]=10; C[ 2][2]=10;
    C[ 3][1]=10; C[ 3][2]=10;
    C[ 4][1]=10; C[ 4][2]=10;

    for(i=4; i!=0; i--) // <-- BUG IS HERE
    {
      /* the calculation of the address of C[i] is pulled out of the loop
         and on top of that the local (out of scope) unsigned char i is used
         instead of the global signed char i */
      C[i][1]=0;
      C[i][2]=0;
    }
  }
}

void testBug(void)
{
  unsigned int i;

  i;

  gm = 0;

  Sub_9000();

  for(i = 1; i <= 4; i++)
    {
      ASSERT(C[i][1] == 0);
      ASSERT(C[i][2] == 0);
    }
}
