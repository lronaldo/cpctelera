#include <stdint.h>
#include <stdio.h>

#define DEVICE DEV_STM8S208

#include "stm8.h"

int putchar(int c)
{
  while(!(USART->sr & USART_SR_TXE));
  USART->dr = c;
  return c;
}

void main(void)
{
  unsigned long i = 0;

  CLK->ckdivr = 0x00; // Set the frequency to 16 MHz
  CLK->pckenr1 = 0xFF; // Enable peripherals

  USART->cr2 = USART_CR2_TEN; // Allow TX and RX
  USART->cr3 &= ~(USART_CR3_STOP1 | USART_CR3_STOP2); // 1 stop bit
  USART->brr2 = 0x03;
  USART->brr1 = 0x68; // 9600 baud

  printf("Hello World!\n");
  for (;;) ;
}
