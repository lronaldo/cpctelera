// Source code under CC0 1.0
#include <stdint.h>
#include <stdio.h>

#include "stm8.h"

#include "serial.h"

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
  
  CLK->ckdivr = 0x00; // Set the frequency to 16 MHz
  CLK->pckenr1 = 0xFF; // Enable peripherals

  USART->cr2 = USART_CR2_TEN | USART_CR2_REN; // Allow TX and RX
  USART->cr3 &= ~(USART_CR3_STOP1 | USART_CR3_STOP2); // 1 stop bit
  USART->brr2 = 0x03;
  USART->brr1 = 0x68; // 9600 baud

  USART->cr2|= USART_CR2_RIEN;
  EI;

  for(;;)
    {
      if (serial_received())
	{
	  char c= getchar();
	  if (c == '=')
	    {
	      print_bl();
	    }
	  else if (c == '*')
	    {
	      printf("0x%04x\n", 0x6000);
	    }
	  else
	    printf("%c", c);
	}
    }
}
