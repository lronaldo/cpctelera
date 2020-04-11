/*-------------------------------------------------------------------------
  support.c - startup for PIC14 regression tests with gpsim
  
  Copyright (c) 2006-2010 Borut Razem
    
  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 2, or (at your option) any
  later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
   
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

  In other words, you are welcome to use, share and improve this program.
  You are forbidden to forbid anyone else to use, share and improve
  what you give them.   Help stamp out software-hoarding!
-------------------------------------------------------------------------*/

#pragma preproc_asm -

#include <pic16f877.h>

static unsigned int __at(0x2007) _config = _WDT_OFF;

void
_putchar(char c)
{
  while (!TXIF)
    ;
  TXREG = c;
  /* wait until the transmit buffer is empty */
  while (!TRMT)
    ;
}


void
_initEmu(void)
{
  /* load and configure the libgpsim_modules module */
  __asm
    ;; Set frequency to 20MHz
    .direct "e", ".frequency=20e6"
    
    ;; Load the USART library and module
    .direct "e", "module library libgpsim_modules"
    .direct "e", "module load usart U1"

    ;; Define a node
    .direct "e", "node PIC_tx"

    ;; Tie the USART module to the PIC
    .direct "e", "attach PIC_tx portc6 U1.RXPIN"

    ;; Set the USART module's Baud Rate
    .direct "e", "U1.rxbaud = 9600"

    ;; Display the received character on terminal
    .direct "e", "U1.console = true"
  __endasm;

  /* USART initialization */
  PORTC |= 0x40;     // Set TX pin to 1
  TRISC &= ~0x40; // TX pin is output

  //1. Initialize the SPBRG register for the appropriate
  //   baud rate. If a high speed baud rate is desired,
  //   set bit BRGH (Section 16.1).
  BRGH = 1;
  SPBRG = 129;

  //2. Enable the asynchronous serial port by clearing
  //   bit SYNC and setting bit SPEN.
  CREN = 1;
  SPEN = 1;

  //3. If interrupts are desired, set enable bit TXIE.
  //4. If 9-bit transmission is desired, set transmit bit
  //   TX9. Can be used as address/data bit.
  //5. Enable the transmission by setting bit TXEN,
  //   which will also set bit TXIF.
  TXEN = 1;

  //6. If 9-bit transmission is selected, the ninth bit
  //   should be loaded in bit TX9D.
  //7. Load data to the TXREG register (starts
  //   transmission).
}


void
_exitEmu(void)
{
  /* set the breakpoint */
  __asm
   .direct "a", "\"PASSED\""
   goto	$
  __endasm;
}

