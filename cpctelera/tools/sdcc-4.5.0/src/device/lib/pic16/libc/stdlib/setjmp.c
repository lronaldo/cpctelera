/*-------------------------------------------------------------------------
   setjmp.c - source file for ANSI routines setjmp & longjmp

   Copyright (C) 2023, Jonathon Hall <dabigjhall@gmail.com>

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

#include <sdcc-lib.h>
#include <pic18fregs.h>
#define __SDCC_HIDE_LONGJMP
#include <setjmp.h>

extern int _gptrload;
extern int _gptrput2;
extern int _gptrput4;
extern int _gptrget2;
extern int _gptrget4;

// We have to stash the return value outside of the stack in longjmp while
// manipulating the stack.
static int longjmp_rv;

// Jump buffer bytes:
// 0 - STKPTR
// 1 - TOSU
// 2 - TOSH
// 3 - TOSL
// 4 (sm) / 4-5 (lg) - FSR2 (H, L for large)
// 5 (sm) / 6-7 (lg) - FSR1 (H, L for large)

int __setjmp (jmp_buf buf) __naked
{
   (void)buf;
   __asm
      // Load the buf pointer using _gptrload().
      // Since we are not creating a stack frame, buf is at FSR1+{1-3}
      MOVFF _PREINC1, _TBLPTRL
      MOVFF _PREINC1, _TBLPTRH
      MOVFF _PREINC1, _PCLATH
      // Rewind stack to restore buf on the stack (caller cleans stack)
      MOVF _POSTDEC1, F
      MOVF _POSTDEC1, F
      MOVF _POSTDEC1, F

      // Prepare for __gptrput4
      CALL __gptrload

      // _gptrput4 writes 4 bytes from TABLAT:PRODH:PRODL:WREG
      // buf[0] = STKPTR
      MOVF _STKPTR, W
      // buf[1] = TOSU
      MOVFF _TOSU, _PRODL
      // buf[2] = TOSH
      MOVFF _TOSH, _PRODH
      // buf[3] = TOSL
      MOVFF _TOSL, _TABLAT
      CALL __gptrput4

      // __gptrput4 advances the pointer for another call as well
#if defined(__SDCC_MODEL_LARGE)
      // buf[4] = FSR2H
      MOVF _FSR2H, W
      // buf[5] = FSR2L
      MOVFF _FSR2L, _PRODL
      // buf[6] = FSR1H
      MOVFF _FSR1H, _PRODH
      // buf[7] = FSR1L
      MOVFF _FSR1L, _TABLAT
      CALL __gptrput4
#else
      // buf[4] = FSR2L
      MOVF _FSR2L, W
      // buf[5] = FSR1L
      MOVFF _FSR1L, _PRODL
      CALL __gptrput2
#endif

      // Return 0
      CLRF _PRODL
      RETLW 0
   __endasm;
}

int longjmp (jmp_buf buf, int rv) __naked
{
   (void)buf;
   (void)rv;
   __asm
      // Load buf into with _gptrget1()
      MOVFF _PREINC1, _TBLPTRL
      MOVFF _PREINC1, _TBLPTRH
      MOVFF _PREINC1, _PCLATH

      // Stash return value
      MOVFF _PREINC1, _longjmp_rv   // LSB
      MOVFF _PREINC1, (_longjmp_rv+1)  // MSB

      // Prepare for __gptrput4
      CALL __gptrload

      // Read 6 (small) / 8 (large) bytes from buf
      CALL __gptrget4
      // buf[0] = STKPTR
      MOVWF _STKPTR
      // buf[1] = TOSU
      MOVFF _PRODL, _TOSU
      // buf[2] = TOSH
      MOVFF _PRODH, _TOSH
      // buf[3] = TOSL
      MOVFF _TABLAT, _TOSL

      // __gptrget4 advances the pointer for another call as well
#if defined(__SDCC_MODEL_LARGE)
      CALL __gptrget4
      // buf[4] = FSR2H
      MOVWF _FSR2H
      // buf[5] = FSR2L
      MOVFF _PRODL, _FSR2L
      // buf[6] = FSR1H
      MOVFF _PRODH, _FSR1H
      // buf[7] = FSR1L
      MOVFF _TABLAT, _FSR1L
#else
      CALL __gptrget2
      // buf[4] = FSR2L
      MOVWF _FSR2L
      // buf[5] = FSR1L
      MOVFF _PRODL, _FSR1L
#endif

      // Put the return value in PRODL:WREG
      MOVF _longjmp_rv, W
      MOVFF (_longjmp_rv+1), _PRODL

      // If either PRODL or WREG is nonzero, return as-is, otherwise return 1
      TSTFSZ _PRODL
      RETURN
      TSTFSZ _WREG
      RETURN
      RETLW 1  // PRODL:WREG are zero, return with WREG=1
   __endasm;
}
