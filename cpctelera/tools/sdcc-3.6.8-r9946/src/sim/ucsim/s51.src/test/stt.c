#include "hw.h"
#include "timer.h"
#include "t0.h"

unsigned char __xdata * volatile sif;

volatile unsigned int tr;

void t0_isr(void) __interrupt (1)
{
  T0set(tr);
}

int var;
__data  int dvar;
__idata int ivar;
__xdata int xvar;
__pdata int pvar;
__code  int cvar;
__sfr   __at(0x81) svar;
__bit   bvar;

void
fn(void)
{
  volatile int fn_local;
  volatile int fn_hide;
  fn_local= fn_hide= 1;
  {
    volatile int fn_blocked, fn_hide;
    fn_blocked= fn_hide= 2;
  }
}

void
sfun(void)
{
  return ;
}

void
main(void)
{
  volatile unsigned int i, x= 0, j;
  __xdata char *p= (__xdata char *)0;

  bvar= 1;
  sif= (unsigned char __xdata *)0xffff;
  tr= T_kezdoertek(10);
  T0idozito1(0);
  T0beallit(tr);
  T0start();
  ET0= 1;
  EA= 1;

  for (j=0; j<10; j++)
    for (i= 0; i<0xfff0; i++)
      {
	char c= p[i];
	unsigned int t= TL0 + (TH0*256);
	p[i]= t&0xff;
	if (p[i] != t&0xff)
	  x++;
	p[i]= c;
      }
  * (char __idata *) 0 = * (char __xdata *) 0xfffe;
  *sif= 's';
}
