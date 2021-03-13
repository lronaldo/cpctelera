// Source code under CC0 1.0
#include <stdint.h>

#include "stm8.h"

#define PC GPIOC
#define PE GPIOE

unsigned int clock(void)
{
  unsigned char h, l;
  h = TIM1->cntrh;
  l = TIM1->cntrl;
  return((unsigned int)(h) << 8 | l);
}

void main(void)
{
  CLK->ckdivr = 0x00; // Set the frequency to 16 MHz
  CLK->pckenr2 |= 0x02; // Enable clock to timer

  // Configure timer
  // 1000 ticks per second
  TIM1->pscrh = 0x3e;
  TIM1->pscrl = 0x80;
  // Enable timer
  TIM1->cr1 = TIM_CR1_CEN;

  // Configure pins
  PE->ddr = 0x80;
  PE->cr1 = 0x80;

  PC->ddr = 0x80;
  PC->cr1 = 0x80;

  for(;;)
    {
      if (clock() % 1000 <= 500)
	PE->odr |= 0x80;
      else
	PE->odr &= 0x7f;
      if (clock() % 2000 <= 1000)
	PC->odr |= 0x80;
      else
	PC->odr &= 0x7f;
    }
}
