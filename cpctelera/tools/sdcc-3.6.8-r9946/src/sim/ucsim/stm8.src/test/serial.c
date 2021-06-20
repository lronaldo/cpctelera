#include <stdio.h>

#include "stm8.h"

#include "serial.h"

int
putchar(int c)
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

char
serial_received()
{
  //return UART2_SR & UART_SR_RXNE;
  return first_free != last_used;
}

char getchar()
{
  uint8_t o;
  while (!serial_received())
    ;
  o= last_used;
  last_used= (last_used+1)%8;
  return rx_buf[o];
}

void
print_cx1(char c)
{
  if (c > 9)
    printf("%c", c-10+'A');
  else
    printf("%c", c+'0');
}

void
print_cx2(char c)
{
  int d;
  d= (c>>4) & 0xf;
  print_cx1(d);
  d= c & 0xf;
  print_cx1(d);
}

void
print_ix4(int i)
{
  int d= i>>12;
  print_cx1(d);
  d= (i>>8) & 0xf;
  print_cx1(d);
  d= (i>>4) & 0xf;
  print_cx1(d);
  d= i & 0xf;
  print_cx1(d);
}
