// Source code under CC0 1.0
#include <stdint.h>
#include <stdio.h>

#include "stm8.h"


int putchar(int c)
{
  while(!(USART->sr & USART_SR_TXE));
  
  USART->dr = c;
  return c;
}


volatile uint8_t rx_buf[8];
volatile uint8_t first_free= 0;
volatile uint8_t last_used= 0;

void isr_rx(void) __interrupt(USART_RX_IRQ)
{
  volatile uint8_t d;
  if (USART->sr & USART_SR_RXNE)
    {
      uint8_t n;
      d= USART->dr;
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

void main(void)
{
  unsigned long i = 0;
  int a= 0;
  
  CLK->ckdivr = 0x00; // Set the frequency to 16 MHz
  CLK->pckenr1 = 0xFF; // Enable peripherals

  GPIOC->ddr = 0x08; // Put TX line on
  GPIOC->cr1 = 0x08;

  USART->cr2 = USART_CR2_TEN | USART_CR2_REN; // Allow TX and RX
  USART->cr3 &= ~(USART_CR3_STOP1 | USART_CR3_STOP2); // 1 stop bit
  USART->brr2 = 0x03;
  USART->brr1 = 0x68; // 9600 baud

  USART->cr2|= USART_CR2_RIEN;
  EI;

  for(;;)
    {
      i++;
      if (received())
	{
	  char c= getchar();
	  if (c == '*')
	    {
	      printf("0x%04x\n", a);
	    }
	  else if (c == '?')
	    {
	      for (i= 0; i < 12; i++)
		{
		  printf("%02x", ((uint8_t*)0x4926)[i]);
		}
	      printf("\n");
	    }
	  else
	    printf("%c", c);
	  i= 0;
	}
      else
      if (i > 147456*2)
	{
	  printf("\ntick %d 0x%04x, press any key\n", a, a);
	  i= 0;
	  a++;
	}
    }
}
