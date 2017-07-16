/*------------------------------------------------------------------------
 test3.c - A simple multiple module example, uses itoa() and crc()
  routines from other modules in same directory.  Compile with:
    make -f maketest3
|------------------------------------------------------------------------*/
#if defined(__mcs51)
#include <8052.h>
#endif
#if defined(__ds390)
#include <ds390.h>
#endif
#include <stdio.h>

typedef unsigned char byte;
typedef unsigned int word;
typedef unsigned long l_word;

void uitoa(unsigned int value, char* string, int radix);
void itoa(int value, char* string, int radix);
byte accum_checksum(byte cs, byte val);

char tmpstr[30];

int i;

#if defined(__mcs51) || defined(__ds390)
/*------------------------------------------------------------------------
  tx_char - transmit(tx) a char out the serial uart.
|------------------------------------------------------------------------*/
void tx_char(char c)
{
  SBUF = c;
  while (!TI)
    ;
  TI = 0;
}

/*------------------------------------------------------------------------
  my_puts - 
|------------------------------------------------------------------------*/
void my_puts(char *str)
{
  
  while (*str)
    tx_char(*str++);
}

/*------------------------------------------------------------------------
  init_mcs51 - 
|------------------------------------------------------------------------*/
void init_mcs51(void)
{
  PCON = 0x80;  /* power control byte, set SMOD bit for serial port */
  SCON = 0x50;  /* serial control byte, mode 1, RI active */
  TMOD = 0x21;  /* timer control mode, byte operation */
  TCON = 0;     /* timer control register, byte operation */

  TH1 = 0xFA;   /* serial reload value, 9,600 baud at 11.0952Mhz */
  TR1 = 1;      /* start serial timer */

  EA = 1;       /* Enable Interrupts */

  TI = 0;       /* clear this out */
}
#else
/*------------------------------------------------------------------------
  my_puts - 
|------------------------------------------------------------------------*/
void my_puts(char *str)
{
  
  while (*str)
    putchar(*str++);  /* putchar() is lib routine which calls simulator trap */
}
#endif

/*------------------------------------------------------------------------
  main - Simple test program to send out something to the serial port.
|------------------------------------------------------------------------*/
int main(void)
{
  byte ccs;
  unsigned int tmp;

#if defined(__mcs51) || defined(__ds390)
  init_mcs51();
#endif

  ccs = 0x3f;
  tmp = (ccs<<7);
  my_puts("0x3f<<7 hex=");
  uitoa(tmp, tmpstr, 10);
  my_puts(tmpstr);
  my_puts("\n");

#if 0
  byte r,cs,val;
  unsigned int tmp;
  cs = 0xff;
  val = 0;
  tmp = ((cs<<7) | (cs>>1)) + val;
  printf("tmp=%xH(0x7fff)\n", tmp);
  cs = 0xfd;
  tmp = ((cs<<7) | (cs>>1)) + val;
  printf("tmp=%xH(0x7efe)\n", tmp);
  //	return (byte)tmp + ((byte) (tmp>>8) & 1);
#endif


  my_puts("Test3 - multiple module test\n");

  my_puts("1023 decimal=");
  uitoa(1023, tmpstr, 10);
  my_puts(tmpstr);
  my_puts(" hex=");
  uitoa(1023, tmpstr, 0x10);
  my_puts(tmpstr);
  my_puts("\n");

  my_puts(" chksum=");
//  i = accum_checksum(50, 20);
  uitoa(i, tmpstr, 10);
  my_puts(tmpstr);
  my_puts("\n");

  for (;;)
  {
  }
  return 0;
}

