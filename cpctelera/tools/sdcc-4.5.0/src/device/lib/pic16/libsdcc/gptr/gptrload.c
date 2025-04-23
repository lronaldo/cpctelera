/*-------------------------------------------------------------------------
   gptrload.c - load generic pointer for calls to _gptrput1-4/_gptrget1-4

   Copyright (C) 2024 Jonathon Hall <dabigjhall@gmail.com>

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

/* sdcc generates code to read or write generic pointers using _gptrload(),
 * _gptrput{1-4}(), and _gptrget{1-4}():
 * 1. Load the generic pointer using _gptrload()
 * 2. For values >4 bytes, call _gptrget4() / _gptrload4() as many times as
 *    needed until 4 or fewer bytes remain
 * 3. Finally, call _gptrget{1-4}() or _gptrput{1-4}() for the last group of 1-4
 *    bytes
 *
 * _gptrload() loads a generic pointer from PCLATH:TBLPTRH:TBLPTRL into the
 * type-specific registers (TBLPTRU-L, FSR0H-L, EEADRH:EEADR).  After that,
 * _gptrget{1-4}() and _gptrput{1-4}() test only PCLATH to determine the pointer
 * type, then read using their type-specific address registers.  _gptrget4() and
 * _gptrput4() leave the next address in the type-specific registers for the
 * next call.
 *
 * This structure leverages the automatic incrementing reads available for
 * RAM and code and avoids moving the address around or reloading it for
 * accesses longer than 4 bytes.  This is important when the address is a
 * temporary, as it could overlap the registers allocated for the result.
 */

extern int FSR0L;
extern int FSR0H;
extern int WREG;
extern int PCLATH;
extern int PRODL;
extern int TBLPTRL;
extern int TBLPTRH;
extern int TBLPTRU;
extern int __eeprom_gptrload;

/* _gptrload() - load a generic pointer for later calls to _gptrget{1-4}() or
 * _gptrput{1-4}(). Call with the generic pointer in PCLATH:TBLPTRH:TBLPTRL. */
void _gptrload(void) __naked
{
  __asm
    /* decode generic pointer MSB (in PCLATH) bits 6 and 7:
     * 00 -> code (unimplemented)
     * 01 -> EEPROM
     * 10 -> data
     * 11 -> data
     */
    btfss   _PCLATH, 7
    bra     _lab_01_
    /* data */
    movff _TBLPTRH, _FSR0H
    movff _TBLPTRL, _FSR0L
    return

_lab_01_:
    /* code or eeprom */
    btfsc   _PCLATH, 6
    goto    ___eeprom_gptrload

    /* code */
    movff _PCLATH, _TBLPTRU

    return

  __endasm;
}
