#include "hw.h"

#if defined __SDCC || defined SDCC
#elif defined __C51__

#else /* IAR4 */
#include <stdio.h>
#endif

#include "serial.h"


volatile unsigned char serial_buffer[SERIAL_BUFFER_SIZE];
volatile unsigned char first_occupied, first_free;
volatile bit serial_sent;


/* Serial line ISR puts received chars into a ring buffer */

#if defined __SDCC || defined SDCC
void serial_isr(void) __interrupt (4)
#elif defined __C51__
void serial_isr(void) interrupt 4
#else /* IAR4 */
interrupt void SCON_int(void)
#endif
{
  if (RI)
    {
      unsigned char new= first_free+1;
      new= new % SERIAL_BUFFER_SIZE;
      if (new != first_occupied)
	{
	  serial_buffer[first_free]= SBUF;
	  first_free= new;
	}
      RI= 0;
    }
  else if (TI)
    {
      serial_sent= 1;
      TI= 0;
    }
}


#if defined __SDCC || defined SDCC
static __sfr __at(0x97) s97;
#elif defined __C51__
static sfr s97= 0x97;
#endif

/* Initialization of serial line */

void
serial_init(long int br)
{
  /* Set  variables */
  first_free= first_occupied= 0;
  if (!(serial_sent= 1))
    serial_dummy();
  
  /* Set USART mode: 8 bit variable speed */
  SCON= 0x40;
  ES= 1;
  REN= 1;

  s97= 0xf0;
  s97= 0x55;
  if (s97 == 0x55)
    {
      beallitas();
      return;
    }

  /* Set Timer2 as baudrate generator, XTAL=11.0592MHz */
  C_T2= 0;
  CP_RL2= 0;
  {
    long int l= (3*115200)/br;
    TL2= RCAP2L= (65536-l)&0xff; /* 3=115200,6=57600,9=38400,18=19200,36=9600 */
    TH2= RCAP2H= (65536-l)/256;
  }
  RCLK= 1;
  TCLK= 1;
  TR2= 1;

  /* Start */
  EA= 1; /* Enable interrupts */
}


void
beallitas(void)
{
  /* Valtozok beallitasa */
  if (first_occupied= 0)
    ;/*serial_dummy();*/
  first_free= 0;
  serial_sent= 1;

  /* Timer1 beallitasa 9600 baud-os sebesseg generalasahoz */
  TMOD= (TMOD & 0x0f) | 0x20;
  TH1= 0xfd;
  TL1= 0xfd;
  PCON&= 0x7f; /* SMOD=0 */
  ET1= 0;
  
  /* Soros vonal beallitasa, 8 bites valtoztathato sebessegu mod */
  SCON= 0x40;
  ES= 1;
  REN= 1;
  
  /* Start */
  TR1= 1;	/* Idozito indul */
  EA= 1;	/* Megszakitasok engedelyezese */
}


/************************************************** Low level line handling */

/* Blocking send */

unsigned char
serial_send(unsigned char c)
{
  while (!serial_sent) ;
  SBUF= c;
  serial_sent= 0;
  return c;
}

void
putchar(char c)
{
  serial_send(c);
}


/* Check for recived chars */

unsigned char
serial_received(void)
{
  return first_free != first_occupied;
}


/* Blocking receive */

unsigned char
serial_receive(void)
{
  unsigned char c;
  
  while (!serial_received()) ;
  ES= 0;
  c= serial_buffer[first_occupied++];
  first_occupied%= SERIAL_BUFFER_SIZE;
  ES= 1;
  return c;
}

char
getchar(void)
{
  return serial_send(serial_receive());
}


void
serial_dummy(void)
{
  serial_init(0);
  beallitas();
  serial_send(0);
  serial_received();
  putchar(0);
  getchar();
}


/* End of serial.c */
