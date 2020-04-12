#include <stdio.h>

unsigned char __xdata * volatile sif;

int putchar(int c)
{
  *sif='p';*sif=c;
}

void
main(void)
{
  volatile unsigned int i, x, j;
  __xdata char *p= (__xdata char *)0;

  sif= (unsigned char __xdata *)0xffff;
  for (j= 0; j<10; j++)
    for (i= 0; i<0xfff0; i++)
      {
	//printf("j=%4x i=%4x\n", j, i);
	x= p[i];
      }
  * (char __idata *) 0 = * (char __xdata *) 0xfffe;
  *sif= 's';
}
