/*-------------------------------------------------------------------------
   gptrget2.c - get 2 byte value from generic pointer

   Copyright (C) 1999, Sandeep Dutta . sandeep.dutta@usa.net
   Adopted for pic16 port by Vangelis Rokas, 2004 <vrokas AT otenet.gr>

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

extern int POSTINC0;
extern int INDF0;
extern int FSR0L;
extern int FSR0H;
extern int WREG;
extern int TBLPTRL;
extern int TBLPTRH;
extern int TBLPTRU;
extern int TABLAT;
extern int PCLATH;
extern int PRODL;
extern int PRODH;
extern int __eeprom_gptrget2;

/* Read 2 bytes following _gptrload().  Result in PRODL:WREG. */
void _gptrget2(void) __naked
{
  __asm
    /* decode generic pointer MSB (in PCLATH) bits 6 and 7:
     * 00 -> code
     * 01 -> EEPROM
     * 10 -> data
     * 11 -> data
     */
    btfss	_PCLATH, 7, 0
    bra		_lab_01_
    
    /* data pointer  */
    movf	_POSTINC0, 0, 0
    movff	_POSTINC0, _PRODL
    
    return
    

_lab_01_:
    /* code or eeprom */
    btfsc	_PCLATH, 6, 0
    goto        ___eeprom_gptrget2
    
    /* code pointer */

    /* fetch first byte */
    TBLRD*+
    movf	_TABLAT, 0, 0

    /* fetch second byte  */
    TBLRD*+
    movff	_TABLAT, _PRODL
    
    return 

  __endasm;
}
