// Source code under CC0 1.0
#include <stdint.h>

#include "stm8.h"

#define PD GPIOD

volatile unsigned long clk= 0;

void tim1_up_isr(void) __interrupt(TIM1_UP_IRQ)
{
  TIM1->sr1&= ~TIM_SR1_UIF;
  clk++;
  //PE->odr^= 0x80;
}

unsigned int clock(void)
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
      PD->odr^= 1;
    }
}

void main(void)
{
  CLK->ckdivr = 0x00; // Set the frequency to 16 MHz

  // Configure timer
  // 1000 ticks per second
  TIM1->pscrh = 0; //0x3e;
  TIM1->pscrl = 0; //0x80;
  // Update event at every 1 ms (16000 count)
  #define AR 16000
  TIM1->arrh = AR >> 8;
  TIM1->arrl = AR & 0xff;
  // Enable timer
  TIM1->cr1 = 0x01;
  
  // Enable interrupt for timer1 update
  TIM1->ier|= TIM_IER_UIE;
  EI;

  // Configure pin
  PD->ddr = 0x01;
  PD->cr1 = 0x01;

  for(;;)
    {
      unsigned long c= clock();
      tick1(c);
    }
}
