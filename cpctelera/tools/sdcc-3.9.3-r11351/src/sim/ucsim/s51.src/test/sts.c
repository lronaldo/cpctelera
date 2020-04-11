#include <stdio.h>
#include "serial.h"
#include "print.h"

unsigned char __xdata * volatile sif;

void
main(void)
{
  volatile unsigned int i;
  __code char *p= (__code char *)0;

  sif= (unsigned char __xdata *)0xffff;
  serial_init(19200);
  for (i= 1; i<0x4000; i++)
    {
      print_cx(p[i]);
      putchar('\n');
    }
  * (char __idata *) 0 = * (char __xdata *) 0xfffe;
  *sif= 's';
}
