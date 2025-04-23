; -------------------------------------------------------------------------
;  _gptrget4.S - read four bytes pointed to by a generic pointer
;
;  Copyright (C) 2005, Raphael Neider <rneider AT web.de>
;
;  This library is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundation; either version 2, or (at your option) any
;  later version.
;
;  This library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License 
;  along with this library; see the file COPYING. If not, write to the
;  Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
;   MA 02110-1301, USA.
;
;  As a special exception, if you link this library with other files,
;  some of which are compiled with SDCC, to produce an executable,
;  this library does not by itself cause the resulting executable to
;  be covered by the GNU General Public License. This exception does
;  not however invalidate any other reasons why the executable file
;  might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

; calling conventions:
;   3 byte generic pointer is passed in via (WREG STK00 STK01).
;   The result is returned in (WREG (STK00 (STK01 (STK02)))).
; 
;   unsigned char _gptrget  (void *gptr);
;   unsigned char _gptrget1 (void *gptr);
;   unsigned int  _gptrget2 (void *gptr);
;   void *        _gptrget3 (void *gptr);
;   unsigned long _gptrget4 (void *gptr);
; 
;   void _gptrput  (void *ptr, unsigned char val);
;   void _gptrput1 (void *ptr, unsigned char val);
;   void _gptrput2 (void *ptr, unsigned int  val);
;   void _gptrput3 (void *ptr, unsigned int  val);
;   void _gptrput4 (void *ptr, unsigned long val);

include macros.inc
include p16f_common.inc

	global	__gptrget4
	extern	__codeptrget1
	
	CODE

__gptrget4:
	select_routine __dataptrget4, __codeptrget4
	; invalid tag -- return 0x00000000
	clrf	STK02
	clrf	STK01
	clrf	STK00
	retlw	0x00

__dataptrget4:
	setup_fsr
	movf	_INDF, W
	movwf	STK02		; LSB in STK02
	inc_fsr
	movf	_INDF, W
	movwf	STK01		; 2nd byte in STK01
	inc_fsr
	movf	_INDF, W
	movwf	STK00		; 3rd byte in STK00
	inc_fsr
	movf	_INDF, W	; MSB in WREG
	return

__codeptrget4:
	pagesel	__codeptrget1	; might reside in different page
	call	__codeptrget1
	movwf	STK02		; temporarily store LSB
	incfsz	STK01,F	; increment low address byte
	decf	STK00,F	; undo increment of high address byte if low byte did not overflow
	incf	STK00,F	; increment high address byte
	pagesel	__codeptrget1	; might reside in different page
	call	__codeptrget1
	movwf	STK03		; temporarily store 2nd byte
	incfsz	STK01,F	; increment low address byte
	decf	STK00,F	; undo increment of high address byte if low byte did not overflow
	incf	STK00,F	; increment high address byte
	pagesel	__codeptrget1	; might reside in different page
	call	__codeptrget1
	movwf	STK04		; temporarily store 3rd byte
	incfsz	STK01,F	; increment low address byte
	decf	STK00,F	; undo increment of high address byte if low byte did not overflow
	incf	STK00,F	; increment high address byte
	pagesel	__codeptrget1	; might reside in different page
	call	__codeptrget1
	movwf	STK05		; temporarily store MSB
	movf	STK02, W
	movwf	STK02		; LSB in STK02
	movf	STK03, W
	movwf	STK01		; 2nd byte in STK01
	movf	STK04, W
	movwf	STK00		; 3rd byte in STK00
	movf	STK05, W	; MSB in WREG
	return

	END
