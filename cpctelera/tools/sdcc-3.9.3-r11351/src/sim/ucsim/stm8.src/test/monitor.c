// Source code under CC0 1.0
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "stm8.h"

#include "serial.h"
#include "flash.h"

#if (DEVICE & DEV_SDISC)
#define LED_PORT GPIOD
#define LED_MASK 0x01
#else
#define LED_PORT GPIOC
#define LED_MASK 0x80
#endif

volatile unsigned char *sif= (unsigned char *)0x7fff;

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

void
dump(unsigned long int start, unsigned long int len)
{
  uint8_t *p= (uint8_t *)0;
  int i= 0;
  
  printf("%06lx ", start);
  while (i<len)
    {
      printf("%02x ", p[start+i]);
      i++;
      if ((i<len) && ((start+i)%8 == 0))
	{
	  printf("\n%06lx ", start+i);
	}
    }
  printf("\n");
}

#define DELIM " ,"

int
xtoi(char *s)
{
  int r= 0;
  while (*s)
    {
      if (isdigit(*s))
	r= r*16 + *s-'0';
      else if ((*s >= 'a') && (*s <= 'f'))
	r= r*16 + *s-'a'+10;
      else if ((*s >= 'A') && (*s <= 'F'))
	r= r*16 + *s-'A'+10;
      s++;
    }
  return r;
}

unsigned long int
xtol(char *s)
{
  unsigned long int r= 0;
  while (*s)
    {
      if (isdigit(*s))
	r= r*16 + *s-'0';
      else if ((*s >= 'a') && (*s <= 'f'))
	r= r*16 + *s-'a'+10;
      else if ((*s >= 'A') && (*s <= 'F'))
	r= r*16 + *s-'A'+10;
      s++;
    }
  return r;
}

void f1() {}
void f2() {}

void
proc_cmd(char *cmd)
{
  char *w= strtok(cmd, DELIM);
  char *s;
  uint8_t res;
  uint8_t *rom= (uint8_t *)0;
  unsigned long addr;
  
  if (w)
    {
      if (strcmp(w, "bl") == 0)
	print_bl();
      else if (strcmp(w, "uid") == 0)
	{
#if defined UID
	  int i;
	  uint8_t *p= UID;
	  printf("0x%04x ", p);
	  for (i= 0; i < 12; i++)
	    printf("%02x ", p[i]);
	  printf("\n");
#else
	  printf("no uid\n");
#endif
	}
      else if (strstr(w, "dump") == w)
	{
	  s= strtok(NULL, DELIM);
	  if (s)
	    {
	      unsigned long int start= xtol(s);
	      unsigned long int len= 32;
	      s= strtok(NULL, DELIM);
	      if (s)
		len= xtol(s);
	      dump(start, len);
	    }
	}
      else if (strcmp(w, "fb") == 0)
	{
	  addr= 0xa000;
	  printf("Before:\n");
	  dump(addr, 1);
	  f1();
	  flash_byte_mode();
	  flash_punlock();
	  rom[addr]= 0xa5;
	  res= flash_wait_finish();
	  f2();
	  flash_plock();
	  printf("After (%s,%d):\n", (res==0)?"succ":"fail", res);
	  dump(addr, 1);
	}
      else if (strcmp(w, "fw") == 0)
	{
	  LED_PORT->odr|= LED_MASK;
	  addr= 0xa0a0;
	  printf("Before:\n");
	  dump(addr, 4);
	  f1();
	  flash_word_mode();
	  flash_punlock();
	  rom[addr+0]= 0x12;
	  rom[addr+1]= 0x34;
	  rom[addr+2]= 0x56;
	  rom[addr+3]= 0x78;
	  res= flash_wait_finish();
	  f2();
	  flash_plock();
	  printf("After (%s,%d):\n", (res==0)?"succ":"fail", res);
	  dump(addr, 4);
	  LED_PORT->odr&= ~LED_MASK;
	}
      else if (strcmp(w, "fe") == 0)
	{
	  LED_PORT->odr|= LED_MASK;
	  addr= 0xa000;
	  printf("Before:\n");
	  dump(addr, 64);
	  f1();
	  flash_punlock();
	  /*
	  rom[addr+0]= 0;
	  rom[addr+1]= 0;
	  rom[addr+2]= 0;
	  rom[addr+3]= 0;
	  res= flash_wait_finish();
	  */
	  res= flash_erase((uint8_t*)0xa000, &(FLASH->iapsr));	  
	  f2();
	  flash_plock();
	  printf("After (%s,%d):\n", (res==0)?"succ":"fail", res);
	  dump(addr, 64);
	  LED_PORT->odr&= ~LED_MASK;
	}
      else if (strstr(w, "test") == w)
	{
	  printf("%d\n", sizeof(flash_erase));
	}
      else
	printf("Unknown command: \"%s\"\n", w);
    }
  else
    printf("What?\n");
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
  unsigned int a= 0;

  CLK->ckdivr = 0x00; // Set the frequency to 16 MHz
  CLK->pckenr1 = 0xFF; // Enable peripherals

  LED_PORT->ddr= LED_MASK;
  LED_PORT->cr1= LED_MASK;
  
  // USART2
  // TX: PD5, CN4.10
  // RX: PD6, CN4.11
  USART->cr2 = USART_CR2_TEN | USART_CR2_REN; // Allow TX and RX
  USART->cr3 &= ~(USART_CR3_STOP1 | USART_CR3_STOP2); // 1 stop bit
  // 0 68 3 0x0683=1667 16MHz-> 9600 baud
  //USART->brr2 = 0x03;
  //USART->brr1 = 0x68;
  // 0 08 b 0x008b=139  16MHz-> 115200 baud
  USART->brr2 = 0x0b;
  USART->brr1 = 0x08;

  USART->cr2|= USART_CR2_RIEN;
  EI;

  printf("%d discovery monitor\n", DEVICE);
  /*{
      uint8_t *p= (uint8_t *)0x123456;
      printf("sizeof p=%d\n", sizeof(p));
      *p= 0;
      }*/
  cmd[0]= 0;
  {/*
    struct st {
      volatile uint8_t v;
    };
    struct st s;*/
    uint8_t r= FLASH->iapsr;
    while (FLASH->iapsr == 0)
      ;//r= FLASH->iapsr;
  }
  for(;;)
    {
      if (serial_received())
	{
	  char c= getchar();
	  proc_input(c);
	}
    }
}
