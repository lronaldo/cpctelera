#include <stdio.h>

volatile unsigned char c;

int putchar(int ch)
{
  c= ch;
  return ch;
}

void main(void)
{
  printf("A");
  putchar('B');
  for (;;) ;
}
