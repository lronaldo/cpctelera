// Source code under CC0 1.0
#include <stdint.h>
#include <stdio.h>

#define PC_DDR	(*(volatile uint8_t *)0x500c)
#define PC_CR1	(*(volatile uint8_t *)0x500d)

#define CLK_DIVR	(*(volatile uint8_t *)0x50c0)
#define CLK_PCKENR1	(*(volatile uint8_t *)0x50c3)

#define USART1_SR	(*(volatile uint8_t *)0x5230)
#define USART1_DR	(*(volatile uint8_t *)0x5231)
#define USART1_BRR1	(*(volatile uint8_t *)0x5232)
#define USART1_BRR2	(*(volatile uint8_t *)0x5233)
#define USART1_CR2	(*(volatile uint8_t *)0x5235)
#define USART1_CR3	(*(volatile uint8_t *)0x5236)

#define USART_CR2_TEN (1 << 3)
#define USART_CR3_STOP2 (1 << 5)
#define USART_CR3_STOP1 (1 << 4)
#define USART_SR_TXE (1 << 7)

int putchar(int c)
{
  while(!(USART1_SR & USART_SR_TXE));

  USART1_DR = c;
  return c;
}

char dbuf[10];
char xbuf[10];

void main(void)
{
  unsigned long i = 0;
  int a= 0;
	
  CLK_DIVR = 0x00; // Set the frequency to 16 MHz
  CLK_PCKENR1 = 0xFF; // Enable peripherals

  PC_DDR = 0x08; // Put TX line on
  PC_CR1 = 0x08;

  USART1_CR2 = USART_CR2_TEN; // Allow TX and RX
  USART1_CR3 &= ~(USART_CR3_STOP1 | USART_CR3_STOP2); // 1 stop bit
  USART1_BRR2 = 0x03; USART1_BRR1 = 0x68; // 9600 baud

  sprintf(dbuf, "%d", 1234);
  printf("%s\n", dbuf);
  sprintf(xbuf, "%x", 0x1234);
  printf("%s\n", xbuf);
  for(;;)
    {
      printf("Hello World %d %x!\n", a, a);
      for(i = 0; i < 147456; i++); // Sleep
      a++;
    }
}
