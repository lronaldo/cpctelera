#include "hw.h"

unsigned char __xdata * volatile sif;

void
main(void)
{
  volatile unsigned int i, j;
  sif= (unsigned char __xdata *)0xffff;
  for (j= 0; j<10; j++)
    for (i= 0; i<0xfff0; i++)
      {
	P0= P1+1;
	P1++;
	P2= P3+1;
	P3++;
      }
  * (char __idata *) 0 = * (char __xdata *) 0xfffe;
  *sif= 's';
}
