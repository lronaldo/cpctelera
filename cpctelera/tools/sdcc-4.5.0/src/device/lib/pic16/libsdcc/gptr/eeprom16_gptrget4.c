/*-------------------------------------------------------------------------
   eeprom16_gptrget4.c - get 4 byte value from EEPROM via a generic pointer

   Copyright (C) 2012 Raphael Neider <rneider AT web.de>

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

extern int EEADR;
extern int EEADRH;
extern int EECON1;
extern int EEDATA;
extern int FSR0H;
extern int FSR0L;
extern int INTCON;
extern int PRODH;
extern int PRODL;
extern int TBLPTRL;
extern int TBLPTRH;
extern int TABLAT;

/* Load 16-bit EEPROM address from generic pointer.  Generic pointer is in
 * PCLATH:TBLPTRH:TBLPTRL. */
void
__eeprom16_gptrload(void) __naked
{
    __asm
        MOVFF   _TBLPTRL, _EEADR
        MOVFF   _TBLPTRH, _EEADRH
        BCF     _EECON1, 7, 0       ; EEPGD = 0: access EEPROM, not program memory
        BCF     _EECON1, 6, 0       ; CFGS = 0: access EEPROM, not config words
        RETURN
    __endasm;
}

/* Read 4 bytes from 16-bit EEPROM address, increment EEPROM address by 4 for
 * next read.  Result in TABLAT:PRODH:PRODL:WREG. */
void
__eeprom16_gptrget4(void) __naked
{
    __asm
        MOVFF   _INTCON, _TBLPTRL   ; save previous interrupt state
        BCF     _INTCON, 7, 0       ; GIE = 0: disable interrupts

        BSF     _EECON1, 0, 0       ; RD = 1: read EEPROM
        MOVF    _EEDATA, 0, 0       ; W = EEPROM[adr]

        INFSNZ  _EEADR, 1, 0        ; address second byte
        INCF    _EEADRH, 1, 0       ; high address bits
        BSF     _EECON1, 0, 0       ; RD = 1: read EEPROM
        MOVFF   _EEDATA, _PRODL     ; PRODL = EEPROM[adr+1]

        INFSNZ  _EEADR, 1, 0        ; address third byte
        INCF    _EEADRH, 1, 0       ; high address bits
        BSF     _EECON1, 0, 0       ; RD = 1: read EEPROM
        MOVFF   _EEDATA, _PRODH     ; PRODH = EEPROM[adr+2]

        INFSNZ  _EEADR, 1, 0        ; address fourth byte
        INCF    _EEADRH, 1, 0       ; high address bits
        BSF     _EECON1, 0, 0       ; RD = 1: read EEPROM
        MOVFF   _EEDATA, _TABLAT    ; TABLAT = EEPROM[adr+3]

        ; Advance to next byte in case another read is needed (code/RAM get this
        ; for free, EEPROM must do it explicitly)
        INFSNZ  _EEADR, 1, 0
        INCF    _EEADRH, 1, 0

        BTFSC   _TBLPTRL, 7, 0      ; check previous interrupt state
        BSF     _INTCON, 7, 0       ; conditionally re-enable interrupts

        RETURN
    __endasm;
}
