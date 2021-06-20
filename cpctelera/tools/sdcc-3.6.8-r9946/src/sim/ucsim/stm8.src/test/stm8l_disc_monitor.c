// Source code under CC0 1.0
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

#define DELIM " ,"

void
proc_cmd(char *cmd)
{
  int i;
  char *s= strtok(cmd, DELIM);
  
  if (s)
    {
      if (strcmp(s, "bl") == 0)
	print_bl();
      if (strcmp(s, "uid") == 0)
	{
#if defined UID
	  uint8_t *p= UID;
	  printf("0x%04x ", p);
	  for (i= 0; i < 12; i++)
	    printf("%02x ", p[i]);
	  printf("\n");
#else
	  printf("no uid\n");
#endif
	}
    }
}

char cmd[100];

void
proc_input(char c)
{
  int l= strlen(cmd);

  printf("%c", c);
  if ((c == '\n') ||
      (c == '\r'))
    {
      proc_cmd(cmd);
      cmd[0]= 0;
    }
  else
    {
      if (l < 99)
	{
	  cmd[l++]= c;
	  cmd[l]= 0;
	}
    }
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

  cmd[0]= 0;
  for(;;)
    {
      if (serial_received())
	{
	  char c= getchar();
	  proc_input(c);
	}
    }
}
