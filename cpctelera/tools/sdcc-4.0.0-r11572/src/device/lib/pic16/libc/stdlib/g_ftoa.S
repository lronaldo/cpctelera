;--------------------------------------------------------------------------
;  g_ftoa.S - floating point to ascii conversion
;
;  Copyright (C) 2004, George Gallant <ggallant571 AT verizon.net>
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

;--
;
;	File:	ftoa.asm
;	Author:	George Gallant
;	Date:	19OCT04
;
;	This routine provides a floating point to ascii conversion.
;	It was written support the SDCC project.
;
;	SDCC C Syntax:
;
;	extern void g_ftoa(data char *buf, float num, char precision);
;
;	The routine is NOT reenterant but expects the entire parameter list
;	to be placed on the stack.
;
;	Notes:	1. measured 105usec to convert -65535.996 on a 20MHz 18f252
;		2. Software stack can not cross a RAM page boundary
;
;--
		list	r=dec, n=96, st=off, mm=off

		nolist
		include		<p18fxxx.inc>
		list

		udata

		extern	digits

exp:		res	1
man:		res	4
r:		res	5
x:		res	3
bp:		res	2
prec:		res	1
ctr:		res	1

		code

		extern	cvt_dec_word
		global	_g_ftoa

_g_ftoa:	movff	FSR2H,POSTDEC1
		movff	FSR2L,POSTDEC1

		movff	FSR1H,FSR2H
		movff	FSR1L,FSR2L

		movff	exp, POSTDEC1
		movff	man+0, POSTDEC1
		movff	man+1, POSTDEC1
		movff	man+2, POSTDEC1
		movff	man+3, POSTDEC1
		movff	r+0, POSTDEC1
		movff	r+1, POSTDEC1
		movff	r+2, POSTDEC1
		movff	r+3, POSTDEC1
		movff	r+4, POSTDEC1
		movff	x+0, POSTDEC1
		movff	x+1, POSTDEC1
		movff	bp+0, POSTDEC1
		movff	bp+1, POSTDEC1
		movff	prec, POSTDEC1
		movff	ctr, POSTDEC1

		movff	FSR0H,POSTDEC1
		movff	FSR0L,POSTDEC1

		movlw	3
		addwf	FSR2L, f
		btfsc	STATUS,C
		incf	FSR2H, f

		movff	POSTINC2,FSR0L		;get the low byte of buf pointer
		movff	POSTINC2,FSR0H

		movff	POSTINC2,man+0		;get the low byte of float
		movff	POSTINC2,man+1
		movff	POSTINC2,man+2
		movff	POSTINC2,exp

		movff	POSTINC2,prec

		rlcf	man+2,w
		rlcf	exp,f
		bnc	@1
		movlw	'-'
		movwf	POSTINC0

@1:		movff	man+0,r+0
		movff	man+1,r+1
		movff	man+2,r+2
		bsf	r+2,7
		clrf	r+3
		clrf	r+4

;	Shift the mantissa left or right by the expondent

		movf	exp,w			;get the expondent
		sublw	127			;subtract the bais
		bz	@4			;skip shifting if zero
		bn	@3			;shift left if neg

@2:		bcf	STATUS,C		;otherwise, shift right
		rrcf	r+4,f
		rrcf	r+3,f
		rrcf	r+2,f
		rrcf	r+1,f
		rrcf	r+0,f
		decfsz	WREG,w
		bra	@2
		bra	@4

@3:		bcf	STATUS,C
		rlcf	r+0, f
		rlcf	r+1, f
		rlcf	r+2, f
		rlcf	r+3, f
		rlcf	r+4, f
		incfsz	WREG,w
		bra	@3

@4:		rlcf	r+2,w			;extract bit 23
		rlcf	r+3,f			;shift rest of whole number
		rlcf	r+4,f

		movff	r+3,PRODL
		movff	r+4,PRODH
		call	cvt_dec_word

		movlw	'.'
		movwf	POSTINC0

@10:		movlw	0x7F			
		andwf	r+2,f
		clrf	r+3

		movff	r+0,x+0			;temp copy
		movff	r+1,x+1
		movff	r+2,x+2

		bcf	STATUS,C		;mult by 2
		rlcf	r+0,f
		rlcf	r+1,f
		rlcf	r+2,f
		rlcf	r+3,f

		bcf	STATUS,C		;mult by 4
		rlcf	r+0,f
		rlcf	r+1,f
		rlcf	r+2,f
		rlcf	r+3,f

		movf	x+0,w			;mult by 5
		addwf	r+0,f
		movf	x+1,w
		addwfc	r+1,f
		movf	x+2,w
		addwfc	r+2,f
		movlw	0
		addwfc	r+3,f

		rlcf	r+2,w			;div by 0x400000			
		rlcf	r+3,f			;or just extract bits 24-22
		rlcf	WREG,w
		rlcf	r+3,f

		movf	r+3,w			;this is the bcd value
		addlw	0x30			;convert to ascii
		movwf	POSTINC0		;and save in memory

		bcf	STATUS,C		;mult by 2
		rlcf	r+0,f
		rlcf	r+1,f
		rlcf	r+2,f

		decfsz	prec,f
		bra	@10

		clrf	POSTINC0		;pack a nullbyte at the end


		movff	ctr, POSTDEC1
		movff	prec, POSTDEC1
		movff	bp+1, POSTDEC1
		movff	bp+0, POSTDEC1
		movff	x+1, POSTDEC1
		movff	x+0, POSTDEC1
		movff	r+4, POSTDEC1
		movff	r+3, POSTDEC1
		movff	r+2, POSTDEC1
		movff	r+1, POSTDEC1
		movff	r+0, POSTDEC1
 		movff	man+3, POSTDEC1
		movff	man+2, POSTDEC1
		movff	man+1, POSTDEC1
		movff	man+0, POSTDEC1
		movff	exp, POSTDEC1

		movff	PREINC1,FSR0L
		movff	PREINC1,FSR0H

		movff	PREINC1,FSR2L
		movff	PREINC1,FSR2H
		return

		end
