#include <stdio.h>

#include "stm8.h"

#include "serial.h"

extern volatile unsigned char *sif;

int
putchar(int c)
{
  while(!(USART->sr & USART_SR_TXE));
  
  USART->dr = c;
  return c;
}

volatile uint8_t rx_buf[UART_BUF_SIZE];
volatile uint8_t first_free= 0;
volatile uint8_t last_used= 0;

unsigned char
serial_nuof_received()
{
  if (first_free == last_used)
    return 0;
  else if (first_free > last_used)
    return first_free - last_used;
  else
    return UART_BUF_SIZE - (last_used - first_free);
}

void isr_rx(void) __interrupt(USART_RX_IRQ)
{
  volatile uint8_t d;
  if (USART->sr & USART_SR_RXNE)
    {
      uint8_t n;
      d= USART->dr;
      n= (first_free+1)%UART_BUF_SIZE;
      *sif= 'p';*sif= '|';
      *sif= 'p';*sif= d;
      *sif= 'p';*sif= '|';
      if (n != last_used)
	{
	  rx_buf[first_free]= d;
	  first_free= n;
	}
      else
	{
	  *sif= 'p';*sif= '*';
	  *sif= 'p';*sif= d;
	  *sif= 'p';*sif= '*';
	}
      d= serial_nuof_received();
      *sif= 'p';*sif= '/';
      *sif= 'p';*sif= d+'0';
      *sif= 'p';*sif= '/';
    }
}

char
serial_received()
{
  //return UART2_SR & UART_SR_RXNE;
  return first_free != last_used;
}

int getchar()
{
  uint8_t o, nr;
  while (!serial_received())
    ;
  o= last_used;
  last_used= (last_used+1)%UART_BUF_SIZE;
  o= rx_buf[o];
  nr= serial_nuof_received();
  *sif= 'p';*sif= '@';
  *sif= 'p';*sif= nr+'0';
  *sif= 'p';*sif= '@';
  return o;
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
