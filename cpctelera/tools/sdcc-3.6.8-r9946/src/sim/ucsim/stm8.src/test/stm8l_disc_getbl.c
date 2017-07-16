// Source code under CC0 1.0
#include <stdint.h>
#include <stdio.h>

#include "stm8.h"

int putchar(int c)
{
  while(!(
	  USART->sr
	  //USART1_SR
	  & USART_SR_TXE));
  
  USART->dr
    //USART1_DR
    = c;
  return c;
}


volatile uint8_t rx_buf[8];
volatile uint8_t first_free= 0;
volatile uint8_t last_used= 0;

void isr_rx(void) __interrupt(USART_RX_IRQ/*28*/)
{
  volatile uint8_t d;
  if (
      USART->sr
      //USART1_SR
      & USART_SR_RXNE)
    {
      uint8_t n;
      d=
	USART->dr
	//USART1_DR
	;
      n= (first_free+1)%8;
      if (n != last_used)
	{
	  rx_buf[first_free]= d;
	  first_free= n;
	}
    }
}

char received()
{
  return first_free != last_used;
}

char getchar()
{
  uint8_t o;
  while (!received())
    ;
  o= last_used;
  last_used= (last_used+1)%8;
  return rx_buf[o];
}

void
print_bl()
{
  int a, l;
  uint8_t *p= (uint8_t *)0x6000;
  printf("%c\n", 2);
  printf("$A%04x,\n", 0x6000);
  for (a= 0, l= 0; a < 0x800; a++)
    {
      printf("%02x ", p[a]);
      l++;
      if ((l % 16) == 0)
	{
	  l= 0;
	  printf("\n");
	}
    }
  printf("%c\n", 3);
}
	 
void main(void)
{
  unsigned long i = 0;
  
  CLK->ckdivr
    //CLK_CKDIVR
    = 0x00; // Set the frequency to 16 MHz
  CLK->pckenr1
    //CLK_PCKENR1
    = 0xFF; // Enable peripherals

  GPIOC->ddr = 0x08; // Put TX line on
  GPIOC->cr1 = 0x08;

  USART->cr2
    //USART1_CR2
    = USART_CR2_TEN | USART_CR2_REN; // Allow TX and RX
  USART->cr3
    //USART1_CR3
    &= ~(USART_CR3_STOP1 | USART_CR3_STOP2); // 1 stop bit
  USART->brr2
    //USART1_BRR2
    = 0x03;
  USART->brr1
    //USART1_BRR1
    = 0x68; // 9600 baud

  USART->cr2
    //USART1_CR2
    |= USART_CR2_RIEN;
  EI;

  for(;;)
    {
      if (received())
	{
	  char c= getchar();
	  if (c == '=')
	    {
	      print_bl();
	    }
	  else
	    printf("%c", c);
	}
    }
}
