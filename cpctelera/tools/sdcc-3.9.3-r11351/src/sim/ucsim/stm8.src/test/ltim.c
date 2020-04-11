// Source code under CC0 1.0
#include <stdint.h>

#include "stm8.h"

#define PC GPIOC
#define PE GPIOE

volatile unsigned long clk= 0;

void tim1_up_isr(void) __interrupt(TIM1_UP_IRQ)
{
  TIM1->sr1&= ~TIM_SR1_UIF;
  clk++;
  //PE->odr^= 0x80;
}

unsigned long clock(void)
{
  unsigned long c;
  TIM1->ier&= ~TIM_IER_UIE;
  c= clk;
  TIM1->ier|= TIM_IER_UIE;
  return c;
}

unsigned long last_tick1= 0;

void tick1(unsigned long c)
{
  //unsigned long c= clock();
  if (c - last_tick1 > 500)
    {
      last_tick1= c;
      PE->odr^= 0x80;
    }
}

unsigned long last_tick2= 0;

void tick2(unsigned long c)
{
  //unsigned long c= clock();
  if (c - last_tick2 > 1000)
    {
      last_tick2= c;
      PC->odr^= 0x80;
    }
}

void main(void)
{
  CLK->ckdivr = 0x00; // Set the frequency to 16 MHz
  CLK->pckenr2 |= 0x02; // Enable clock to timer

  // Configure timer
  // 16 MHz clock for timer
  TIM1->pscrh = 0;//0x3e;
  TIM1->pscrl = 0;//0x80;
  // Update event at every 1 ms (16000 count)
  #define AR 16000
  TIM1->arrh = AR >> 8;
  TIM1->arrl = AR & 0xff;
  // Enable timer
  TIM1->cr1 = TIM_CR1_CEN;

  // Enable interrupt for timer1 update
  TIM1->ier|= TIM_IER_UIE;
  EI;
  
  // Configure pins
  PE->ddr = 0x80;
  PE->cr1 = 0x80;

  PC->ddr = 0x80;
  PC->cr1 = 0x80;

  for(;;)
    {
      unsigned long c= clock();
      tick1(c);
      tick2(c);
    }
}
