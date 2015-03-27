/*-------------------------------------------------------------------------
   i2copen.c

   Copyright (C) 2005, Vangelis Rokas <vrokas AT otenet.gr>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License 
   along with this library; see the file COPYING. If not, write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA.

   As a special exception, if you link this library with other files,
   some of which are compiled with SDCC, to produce an executable,
   this library does not by itself cause the resulting executable to
   be covered by the GNU General Public License. This exception does
   not however invalidate any other reasons why the executable file
   might be covered by the GNU General Public License.
-------------------------------------------------------------------------*/

#include <pic18fregs.h>
#include <i2c.h>

void i2c_open(unsigned char mode, unsigned char slew, unsigned char addr_brd)
{
  SSPSTAT &= 0x3f;
  SSPCON1 = 0;
  SSPCON2 = 0;
  SSPCON1 |= mode;
  SSPSTAT |= slew;


#if defined(pic18f2455) || defined (pic18f2550) \
    || defined(pic18f4455) || defined (pic18f4550) \
    || defined(pic18f66j60) || defined(pic18f66j65) || defined(pic18f67j60) \
    || defined(pic18f86j60) || defined(pic18f86j65) || defined(pic18f87j60) \
    || defined(pic18f96j60) || defined(pic18f96j65) || defined(pic18f97j60)

  TRISBbits.TRISB1 = 1;
  TRISBbits.TRISB0 = 1;

#elif  defined(pic18f24j50) || defined(pic18f25j50) || defined(pic18f26j50) \
    || defined(pic18f44j50) || defined(pic18f45j50) || defined(pic18f46j50)

  TRISBbits.TRISB4 = 1;
  TRISBbits.TRISB5 = 1;

#else	/* all other devices */

  TRISCbits.TRISC3 = 1;
  TRISCbits.TRISC4 = 1;

#endif

  SSPADD = addr_brd;

  SSPCON1 |= 0x20;
}

