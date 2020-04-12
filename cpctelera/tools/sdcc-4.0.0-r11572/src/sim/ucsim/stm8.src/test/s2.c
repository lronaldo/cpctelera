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

void waitsend()
{
  while(!(USART->sr & USART_SR_TXE));
}

int putchar(int c)
{
  while(!(USART->sr & USART_SR_TXE));
  USART->dr = c;
  return c;
}

char received()
{
  return USART->sr & USART_SR_RXNE;
}

int getchar()
{
  while (!received())
    ;
  return USART->dr;
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

  USART->cr2 = USART_CR2_TEN; // Allow TX only yet
  USART->cr3 &= ~(USART_CR3_STOP1 | USART_CR3_STOP2); // 1 stop bit
  USART->brr2 = 0x03;
  USART->brr1 = 0x68; // 9600 baud
  
  printf("Hello World!\n");
  waitsend();
  USART->cr2 = USART_CR2_TEN | USART_CR2_REN; // Allow TX and RX
  for (;;)
    {
      if (received())
	{
	  char c= getchar();
	  *sif= 'x';*sif= c;
	  putchar(toupper(c));
	  if (c == 'Z')
	    *sif= 's';
	}
    }
}
