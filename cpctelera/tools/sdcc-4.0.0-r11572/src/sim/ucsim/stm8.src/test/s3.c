#include <stdint.h>
#include <stdio.h>
#include <ctype.h>

#define DEVICE DEV_STM8S208

#include "stm8.h"

volatile unsigned char *sif= (unsigned char *)0x7fff;

int sifchar(int c)
{
  *sif= 'p';
  *sif= c;
  return c;
}

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
  *sif='p';*sif='I';
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
  //return UART2_SR & UART_SR_RXNE;
  return first_free != last_used;
}

int getchar()
{
  uint8_t o;
  while (!received())
    ;
  o= last_used;
  last_used= (last_used+1)%8;
  return rx_buf[o];
}

void prints(char *s)
{
  char i= 0;
  while (s[i])
    {
      putchar(s[i]);
      i++;
    }
}


void main(void)
{
  unsigned long i = 0;

  CLK->ckdivr = 0x00; // Set the frequency to 16 MHz
  CLK->pckenr1 = 0xFF; // Enable peripherals

  USART->cr2 = USART_CR2_TEN | USART_CR2_REN; // Allow TX and RX
  USART->cr3 &= ~(USART_CR3_STOP1 | USART_CR3_STOP2); // 1 stop bit
  USART->brr2 = 0x03;
  USART->brr1 = 0x68; // 9600 baud

  USART->cr2|= USART_CR2_RIEN;
  EI;
  
  printf("Hello World!\n");
  for (;;)
    {
      if (received())
	{
	  char c= getchar();
	  *sif= 'x';*sif= c;
	  putchar(toupper(c));
	}
    }
}
