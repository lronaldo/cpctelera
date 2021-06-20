#include <stdio.h>

volatile unsigned char *sif= (unsigned char *)0x7fff;

int putchar(int c)
{
  *sif= 'p';
  *sif= c;
  return c;
}

void main(void)
{
  printf("A");
}
