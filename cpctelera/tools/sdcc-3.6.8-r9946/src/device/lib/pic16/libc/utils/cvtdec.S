;--------------------------------------------------------------------------
;  cvtdec.S - convert a 16-bit binary word to 5 BCD bytes
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
;	File:	cvtdec.asm
;	Author:	George Gallant
;	Date:	19OCT04
;	
;	This routine is based on the code and algorithm by
;	Nikolai Golovchenko and Scott Dattalo presented in
;	the piclist webb site on radix conversion methods.
;	Modified to support integers from 0 to 65535 and coded
;	for the pic18 chip set.
;
;			   Digit
;	BIT  Weight  4   3   2   1   0
;	---  ------ --- --- --- --- ---
;	 0      1    0   0   0   0   1
;	 1      2    0   0   0   0   2
;	 2      4    0   0   0   0   4
;	 3      8    0   0   0   0   8
;	
;	 4     16    0   0   0   1   6
;	 5     32    0   0   0   3   2
;	 6     64    0   0   0   6   4
;	 7    128    0   0   1   2   8
;
;	 8    256    0   0   2   5   6
;	 9    512    0   0   5   1   2
;	10   1024    0   1   0   2   4
;	11   2048    0   2   0   4   8
;
;	12   4096    0   4   0   9   6
;	13   8192    0   8   1   9   2
;	14  16384    1   6   3   8   4
;	15  32768    3   2   7   6   8
;
;--
		nolist
		include	<p18fxxx.inc>
		list

		udata

		global	digits

digits:		res	6

		code

		global	cvt_dec_word

;--
;
;	Convert a 16-bit binary word to 5 BCD bytes
;
;	On Call
;		PROD	16 bit word
;		FSR0	pointer to ascii buffer
;
;	On Return
;		digits[4:0] contain the BCD integers
;
;	Notes:	1. strip leading zeros
;		2. only positive integers on input
;		3. FSR0, PROD & W are modified and not preserved
;
;--
cvt_dec_word:	clrw				;calculate digit 0 - lsd

		btfsc	PRODL,0			;bit0
		addlw	1

		btfsc	PRODL,1			;bit1
		addlw	2

		btfsc	PRODL,2			;bit2
		addlw	4

		btfsc	PRODL,3			;bit3
		addlw	8

		btfsc	PRODL,4			;bit4
		addlw	6

		btfsc	PRODL,5			;bit5
		addlw	2

		btfsc	PRODL,6			;bit6
		addlw	4

		btfsc	PRODL,7			;bit7
		addlw	8

		btfsc	PRODH,0			;bit8
		addlw	6

		btfsc	PRODH,1			;bit9
		addlw	2

		btfsc	PRODH,2			;bit10
		addlw	4

		btfsc	PRODH,3			;bit11
		addlw	8

		btfsc	PRODH,4			;bit12
		addlw	6

		btfsc	PRODH,5			;bit13
		addlw	2

		btfsc	PRODH,6			;bit14
		addlw	4

		btfsc	PRODH,7			;bit15
		addlw	8

		clrf	digits+1		;setup for carry in 10's
@1:		addlw	-10
		bnc	@2
		incf	digits+1,f
		bra	@1

@2:		addlw	10
		movwf	digits+0

;	calculate digit 1

		movf	digits+1,w

		btfsc	PRODL,4			;bit4
		addlw	1

		btfsc	PRODL,5			;bit5
		addlw	3

		btfsc	PRODL,6			;bit6
		addlw	6

		btfsc	PRODL,7			;bit7
		addlw	2

		btfsc	PRODH,0			;bit8
		addlw	5

		btfsc	PRODH,1			;bit9
		addlw	1

		btfsc	PRODH,2			;bit10
		addlw	2

		btfsc	PRODH,3			;bit11
		addlw	4

		btfsc	PRODH,4			;bit12
		addlw	9

		btfsc	PRODH,5			;bit13
		addlw	9

		btfsc	PRODH,6			;bit14
		addlw	8

		btfsc	PRODH,7			;bit15
		addlw	6

		clrf	digits+2		;setup for carry into 100's
@3:		addlw	-10
		bnc	@4
		incf	digits+2,f
		bra	@3

@4:		addlw	10
		movwf	digits+1

;	calculate digit 2

		movf	digits+2,w

		btfsc	PRODL,7			;bit7
		addlw	1

		btfsc	PRODH,0			;bit8
		addlw	2

		btfsc	PRODH,1			;bit9
		addlw	5

		btfsc	PRODH,5			;bit13
		addlw	1

		btfsc	PRODH,6			;bit14
		addlw	3

		btfsc	PRODH,7			;bit15
		addlw	7

		clrf	digits+3		;setup for carry into 1000's
@5:		addlw	-10
		bnc	@6
		incf	digits+3,f
		bra	@5
@6:		addlw	10

		movwf	digits+2

;	calculate digit 3

		movf	digits+3,w

		btfsc	PRODH,2			;bit10
		addlw	1

		btfsc	PRODH,3			;bit11
		addlw	2

		btfsc	PRODH,4			;bit12
		addlw	4

		btfsc	PRODH,5			;bit13
		addlw	8

		btfsc	PRODH,6			;bit14
		addlw	6

		btfsc	PRODH,7			;bit15
		addlw	2

		clrf	digits+4		;setup for carry into 10000's
@7:		addlw	-10
		bnc	@8
		incf	digits+4,f
		bra	@7

@8:		addlw	10
		movwf	digits+3

;	calculate digit 4

		movf	digits+4,w

		btfsc	PRODH,6			;bit14
		addlw	1

		btfsc	PRODH,7			;bit15
		addlw	3

@9:		addlw	-10
		bc	@9

		addlw	10
;		movwf	digits+4

;		movf	digits+4,w
		bnz	@11
		movf	digits+3,w
		bnz	@12
		movf	digits+2,w
		bnz	@13
		movf	digits+1,w
		bnz	@14
		bra	@15

@11:		addlw	0x30
		movwf	POSTINC0

		movf	digits+3,w
@12:		addlw	0x30
		movwf	POSTINC0

		movf	digits+2,w
@13:		addlw	0x30
		movwf	POSTINC0

		movf	digits+1,w
@14:		addlw	0x30
		movwf	POSTINC0

@15:		movf	digits+0,w
		addlw	0x30
		movwf	POSTINC0

		return

		end
