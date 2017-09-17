// Source code under CC0 1.0
#include <stdint.h>
#include <stdio.h>

#include "stm8.h"

#include "serial.h"


void main(void)
{
  unsigned long i = 0;
  unsigned int a= 0;

  CLK->ckdivr = 0x00; // Set the frequency to 16 MHz
  CLK->pckenr1 = 0xFF; // Enable peripherals

  // USART2
  // TX: PD5, CN4.10
  // RX: PD6, CN4.11
  USART->cr2 = USART_CR2_TEN | USART_CR2_REN; // Allow TX and RX
  USART->cr3 &= ~(USART_CR3_STOP1 | USART_CR3_STOP2); // 1 stop bit
  USART->brr2 = 0x03;
  USART->brr1 = 0x68; // 9600 baud

  USART->cr2|= USART_CR2_RIEN;
  EI;

  for(;;)
    {
      i++;
      if (serial_received())
	{
	  char c= getchar();
	  if (c == '*')
	    {
	      printf("0x%04x\n", a);
	    }
	  printf("%c", c);
	  i= 0;
	}
      if (i > 147456*2)
	{
	  printf("tick %u 0x%04x Hello World!\n", a, a);
	  i= 0;
	  a++;
	}
    }
}
