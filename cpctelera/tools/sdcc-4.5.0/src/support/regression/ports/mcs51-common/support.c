/* define UART sfr only */
__sbit __at(0x98+1) TI;
__sfr  __at(0x99) SBUF;

/* assume P1 for bankswitching address lines */
__sfr __at(0x90) PSBANK;

unsigned char
__sdcc_external_startup (void) __nonbanked
{
  /* serial port mode 0 is default */
  /* enable transmission of first byte */
  TI = 1;
  return 0;
}

void
_putchar (char c)
{
  (* (volatile char __xdata *) 0x7654)= 'p';
  (* (volatile char __xdata *) 0x7654)= c;
  return;
  while (!TI)
    ;
  TI = 0;
  SBUF = c;
}

void
_initEmu (void)
{
}

void
_exitEmu (void)
{
  //while (!TI) /* wait for the last character to be transmitted */
  //  ;         /* before hitting the breakpoint */
  * (char __idata *) 0 = * (char __xdata *) 0x7654;
  (* (volatile char __xdata *) 0x7654)= 's';
}
