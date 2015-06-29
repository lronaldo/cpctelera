;--------------------------------------------------------------------------
;  cnvfrac.S - convertion routine of 24 bits floating point to ASCII
;
;  Copyright (C) 2004, Vangelis Rokas <vrokas at otenet.gr>
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

	radix dec
	list
	nolist


WREG	equ	0xfe8
POSTINC0	equ	0xfee
POSTDEC1	equ	0xfe5
PREINC1	equ	0xfe4
STATUS	equ	0xfd8


	global	_convert_frac


.registers	udata_ovr	0x000
r0x00	res	1
r0x01	res	1
r0x02	res	1
r0x03	res	1
round_digit	res	1
d0x00	res	1
d0x01	res	1
d0x02	res	1
d0x03	res	1
d0x04	res	1
d0x05	res	1
d0x06	res	1
d0x07	res	1
d0x08	res	1
d0x09	res	1
d0x10	res	1
d0x11	res	1
d0x12	res	1
d0x13	res	1
d0x14	res	1
d0x15	res	1
d0x16	res	1
d0x17	res	1
d0x18	res	1
d0x19	res	1
d0x20	res	1
d0x21	res	1
d0x22	res	1
d0x23	res	1
d0x24	res	1


S__convert_frac	code
_convert_frac:

	movff	d0x00, POSTDEC1
	movff	d0x01, POSTDEC1
	movff	d0x02, POSTDEC1
	movff	d0x03, POSTDEC1
	movff	d0x04, POSTDEC1
	movff	d0x05, POSTDEC1
	movff	d0x06, POSTDEC1
	movff	d0x07, POSTDEC1
	movff	d0x08, POSTDEC1
	movff	d0x09, POSTDEC1
	movff	d0x10, POSTDEC1
	movff	d0x11, POSTDEC1
	movff	d0x12, POSTDEC1
	movff	d0x13, POSTDEC1
	movff	d0x14, POSTDEC1
	movff	d0x15, POSTDEC1
	movff	d0x16, POSTDEC1
	movff	d0x17, POSTDEC1
	movff	d0x18, POSTDEC1
	movff	d0x19, POSTDEC1
	movff	d0x20, POSTDEC1
	movff	d0x21, POSTDEC1
	movff	d0x22, POSTDEC1
	movff	d0x23, POSTDEC1
	movff	d0x24, POSTDEC1


; adding digit 0
;	movff	0xf7f, 0xfe8

	movlw	0

	btfsc	r0x00, 0		; bit 0
	addlw	5


	clrf	d0x01
@1:
	addlw	-10
	bnc	@2
	incf	d0x01, f
	bra	@1
@2:
	addlw	10
	movwf	d0x00


	decfsz	round_digit, f
	bra	@3
	addlw	251
	bnc	@3
	incf	d0x01, f

@3:

; adding digit 1
	movf	d0x01, w

	btfsc	r0x00, 0		; bit 0
	addlw	2

	btfsc	r0x00, 1		; bit 1
	addlw	5


	clrf	d0x02
@4:
	addlw	-10
	bnc	@5
	incf	d0x02, f
	bra	@4
@5:
	addlw	10
	movwf	d0x01


	decfsz	round_digit, f
	bra	@6
	addlw	251
	bnc	@6
	incf	d0x02, f

@6:

; adding digit 2
	movf	d0x02, w

	btfsc	r0x00, 0		; bit 0
	addlw	6

	btfsc	r0x00, 1		; bit 1
	addlw	2

	btfsc	r0x00, 2		; bit 2
	addlw	5


	clrf	d0x03
@7:
	addlw	-10
	bnc	@8
	incf	d0x03, f
	bra	@7
@8:
	addlw	10
	movwf	d0x02


	decfsz	round_digit, f
	bra	@9
	addlw	251
	bnc	@9
	incf	d0x03, f

@9:

; adding digit 3
	movf	d0x03, w

	btfsc	r0x00, 1		; bit 1
	addlw	1

	btfsc	r0x00, 2		; bit 2
	addlw	2

	btfsc	r0x00, 3		; bit 3
	addlw	5


	clrf	d0x04
@10:
	addlw	-10
	bnc	@11
	incf	d0x04, f
	bra	@10
@11:
	addlw	10
	movwf	d0x03


	decfsz	round_digit, f
	bra	@12
	addlw	251
	bnc	@12
	incf	d0x04, f

@12:

; adding digit 4
	movf	d0x04, w

	btfsc	r0x00, 0		; bit 0
	addlw	9

	btfsc	r0x00, 1		; bit 1
	addlw	8

	btfsc	r0x00, 2		; bit 2
	addlw	6

	btfsc	r0x00, 3		; bit 3
	addlw	2

	btfsc	r0x00, 4		; bit 4
	addlw	5


	clrf	d0x05
@13:
	addlw	-10
	bnc	@14
	incf	d0x05, f
	bra	@13
@14:
	addlw	10
	movwf	d0x04


	decfsz	round_digit, f
	bra	@15
	addlw	251
	bnc	@15
	incf	d0x05, f

@15:

; adding digit 5
	movf	d0x05, w

	btfsc	r0x00, 0		; bit 0
	addlw	3

	btfsc	r0x00, 1		; bit 1
	addlw	7

	btfsc	r0x00, 2		; bit 2
	addlw	5

	btfsc	r0x00, 3		; bit 3
	addlw	1

	btfsc	r0x00, 4		; bit 4
	addlw	2

	btfsc	r0x00, 5		; bit 5
	addlw	5


	clrf	d0x06
@16:
	addlw	-10
	bnc	@17
	incf	d0x06, f
	bra	@16
@17:
	addlw	10
	movwf	d0x05


	decfsz	round_digit, f
	bra	@18
	addlw	251
	bnc	@18
	incf	d0x06, f

@18:

; adding digit 6
	movf	d0x06, w

	btfsc	r0x00, 0		; bit 0
	addlw	5

	btfsc	r0x00, 2		; bit 2
	addlw	1

	btfsc	r0x00, 3		; bit 3
	addlw	3

	btfsc	r0x00, 4		; bit 4
	addlw	6

	btfsc	r0x00, 5		; bit 5
	addlw	2

	btfsc	r0x00, 6		; bit 6
	addlw	5


	clrf	d0x07
@19:
	addlw	-10
	bnc	@20
	incf	d0x07, f
	bra	@19
@20:
	addlw	10
	movwf	d0x06


	decfsz	round_digit, f
	bra	@21
	addlw	251
	bnc	@21
	incf	d0x07, f

@21:

; adding digit 7
	movf	d0x07, w

	btfsc	r0x00, 0		; bit 0
	addlw	7

	btfsc	r0x00, 1		; bit 1
	addlw	5

	btfsc	r0x00, 5		; bit 5
	addlw	1

	btfsc	r0x00, 6		; bit 6
	addlw	2

	btfsc	r0x00, 7		; bit 7
	addlw	5


	clrf	d0x08
@22:
	addlw	-10
	bnc	@23
	incf	d0x08, f
	bra	@22
@23:
	addlw	10
	movwf	d0x07


	decfsz	round_digit, f
	bra	@24
	addlw	251
	bnc	@24
	incf	d0x08, f

@24:

; adding digit 8
	movf	d0x08, w

	btfsc	r0x00, 0		; bit 0
	addlw	7

	btfsc	r0x00, 1		; bit 1
	addlw	5

	btfsc	r0x00, 2		; bit 2
	addlw	1

	btfsc	r0x00, 3		; bit 3
	addlw	2

	btfsc	r0x00, 4		; bit 4
	addlw	4

	btfsc	r0x00, 5		; bit 5
	addlw	8

	btfsc	r0x00, 6		; bit 6
	addlw	6

	btfsc	r0x00, 7		; bit 7
	addlw	2

	btfsc	r0x01, 0		; bit 8
	addlw	5


	clrf	d0x09
@25:
	addlw	-10
	bnc	@26
	incf	d0x09, f
	bra	@25
@26:
	addlw	10
	movwf	d0x08


	decfsz	round_digit, f
	bra	@27
	addlw	251
	bnc	@27
	incf	d0x09, f

@27:

; adding digit 9
	movf	d0x09, w

	btfsc	r0x00, 0		; bit 0
	addlw	4

	btfsc	r0x00, 1		; bit 1
	addlw	9

	btfsc	r0x00, 2		; bit 2
	addlw	9

	btfsc	r0x00, 3		; bit 3
	addlw	8

	btfsc	r0x00, 4		; bit 4
	addlw	6

	btfsc	r0x00, 5		; bit 5
	addlw	2

	btfsc	r0x00, 6		; bit 6
	addlw	5

	btfsc	r0x00, 7		; bit 7
	addlw	1

	btfsc	r0x01, 0		; bit 8
	addlw	2

	btfsc	r0x01, 1		; bit 9
	addlw	5


	clrf	d0x10
@28:
	addlw	-10
	bnc	@29
	incf	d0x10, f
	bra	@28
@29:
	addlw	10
	movwf	d0x09


	decfsz	round_digit, f
	bra	@30
	addlw	251
	bnc	@30
	incf	d0x10, f

@30:

; adding digit 10
	movf	d0x10, w

	btfsc	r0x00, 0		; bit 0
	addlw	4

	btfsc	r0x00, 1		; bit 1
	addlw	8

	btfsc	r0x00, 2		; bit 2
	addlw	7

	btfsc	r0x00, 3		; bit 3
	addlw	5

	btfsc	r0x00, 4		; bit 4
	addlw	1

	btfsc	r0x00, 5		; bit 5
	addlw	3

	btfsc	r0x00, 6		; bit 6
	addlw	6

	btfsc	r0x00, 7		; bit 7
	addlw	3

	btfsc	r0x01, 0		; bit 8
	addlw	6

	btfsc	r0x01, 1		; bit 9
	addlw	2

	btfsc	r0x01, 2		; bit 10
	addlw	5


	clrf	d0x11
@31:
	addlw	-10
	bnc	@32
	incf	d0x11, f
	bra	@31
@32:
	addlw	10
	movwf	d0x10


	decfsz	round_digit, f
	bra	@33
	addlw	251
	bnc	@33
	incf	d0x11, f

@33:

; adding digit 11
	movf	d0x11, w

	btfsc	r0x00, 0		; bit 0
	addlw	6

	btfsc	r0x00, 1		; bit 1
	addlw	2

	btfsc	r0x00, 2		; bit 2
	addlw	5

	btfsc	r0x00, 3		; bit 3
	addlw	1

	btfsc	r0x00, 4		; bit 4
	addlw	3

	btfsc	r0x00, 5		; bit 5
	addlw	6

	btfsc	r0x00, 6		; bit 6
	addlw	2

	btfsc	r0x00, 7		; bit 7
	addlw	5

	btfsc	r0x01, 1		; bit 9
	addlw	1

	btfsc	r0x01, 2		; bit 10
	addlw	2

	btfsc	r0x01, 3		; bit 11
	addlw	5


	clrf	d0x12
@34:
	addlw	-10
	bnc	@35
	incf	d0x12, f
	bra	@34
@35:
	addlw	10
	movwf	d0x11


	decfsz	round_digit, f
	bra	@36
	addlw	251
	bnc	@36
	incf	d0x12, f

@36:

; adding digit 12
	movf	d0x12, w

	btfsc	r0x00, 0		; bit 0
	addlw	4

	btfsc	r0x00, 1		; bit 1
	addlw	9

	btfsc	r0x00, 2		; bit 2
	addlw	8

	btfsc	r0x00, 3		; bit 3
	addlw	7

	btfsc	r0x00, 4		; bit 4
	addlw	4

	btfsc	r0x00, 5		; bit 5
	addlw	8

	btfsc	r0x00, 6		; bit 6
	addlw	7

	btfsc	r0x00, 7		; bit 7
	addlw	4

	btfsc	r0x01, 0		; bit 8
	addlw	9

	btfsc	r0x01, 1		; bit 9
	addlw	8

	btfsc	r0x01, 2		; bit 10
	addlw	6

	btfsc	r0x01, 3		; bit 11
	addlw	2

	btfsc	r0x01, 4		; bit 12
	addlw	5


	clrf	d0x13
@37:
	addlw	-10
	bnc	@38
	incf	d0x13, f
	bra	@37
@38:
	addlw	10
	movwf	d0x12


	decfsz	round_digit, f
	bra	@39
	addlw	251
	bnc	@39
	incf	d0x13, f

@39:

; adding digit 13
	movf	d0x13, w

	btfsc	r0x00, 2		; bit 2
	addlw	1

	btfsc	r0x00, 3		; bit 3
	addlw	3

	btfsc	r0x00, 4		; bit 4
	addlw	7

	btfsc	r0x00, 5		; bit 5
	addlw	4

	btfsc	r0x00, 6		; bit 6
	addlw	9

	btfsc	r0x00, 7		; bit 7
	addlw	9

	btfsc	r0x01, 0		; bit 8
	addlw	8

	btfsc	r0x01, 1		; bit 9
	addlw	7

	btfsc	r0x01, 2		; bit 10
	addlw	5

	btfsc	r0x01, 3		; bit 11
	addlw	1

	btfsc	r0x01, 4		; bit 12
	addlw	2

	btfsc	r0x01, 5		; bit 13
	addlw	5


	clrf	d0x14
@40:
	addlw	-10
	bnc	@41
	incf	d0x14, f
	bra	@40
@41:
	addlw	10
	movwf	d0x13


	decfsz	round_digit, f
	bra	@42
	addlw	251
	bnc	@42
	incf	d0x14, f

@42:

; adding digit 14
	movf	d0x14, w

	btfsc	r0x00, 0		; bit 0
	addlw	6

	btfsc	r0x00, 1		; bit 1
	addlw	2

	btfsc	r0x00, 2		; bit 2
	addlw	4

	btfsc	r0x00, 3		; bit 3
	addlw	8

	btfsc	r0x00, 4		; bit 4
	addlw	6

	btfsc	r0x00, 5		; bit 5
	addlw	3

	btfsc	r0x00, 6		; bit 6
	addlw	6

	btfsc	r0x00, 7		; bit 7
	addlw	3

	btfsc	r0x01, 0		; bit 8
	addlw	7

	btfsc	r0x01, 1		; bit 9
	addlw	5

	btfsc	r0x01, 2		; bit 10
	addlw	1

	btfsc	r0x01, 3		; bit 11
	addlw	3

	btfsc	r0x01, 4		; bit 12
	addlw	6

	btfsc	r0x01, 5		; bit 13
	addlw	2

	btfsc	r0x01, 6		; bit 14
	addlw	5


	clrf	d0x15
@43:
	addlw	-10
	bnc	@44
	incf	d0x15, f
	bra	@43
@44:
	addlw	10
	movwf	d0x14


	decfsz	round_digit, f
	bra	@45
	addlw	251
	bnc	@45
	incf	d0x15, f

@45:

; adding digit 15
	movf	d0x15, w

	btfsc	r0x00, 0		; bit 0
	addlw	9

	btfsc	r0x00, 1		; bit 1
	addlw	9

	btfsc	r0x00, 2		; bit 2
	addlw	8

	btfsc	r0x00, 3		; bit 3
	addlw	6

	btfsc	r0x00, 4		; bit 4
	addlw	3

	btfsc	r0x00, 5		; bit 5
	addlw	7

	btfsc	r0x00, 6		; bit 6
	addlw	4

	btfsc	r0x00, 7		; bit 7
	addlw	9

	btfsc	r0x01, 0		; bit 8
	addlw	8

	btfsc	r0x01, 1		; bit 9
	addlw	7

	btfsc	r0x01, 2		; bit 10
	addlw	5

	btfsc	r0x01, 5		; bit 13
	addlw	1

	btfsc	r0x01, 6		; bit 14
	addlw	2

	btfsc	r0x01, 7		; bit 15
	addlw	5


	clrf	d0x16
@46:
	addlw	-10
	bnc	@47
	incf	d0x16, f
	bra	@46
@47:
	addlw	10
	movwf	d0x15


	decfsz	round_digit, f
	bra	@48
	addlw	251
	bnc	@48
	incf	d0x16, f

@48:

; adding digit 16
	movf	d0x16, w

	btfsc	r0x00, 0		; bit 0
	addlw	5

	btfsc	r0x00, 1		; bit 1
	addlw	1

	btfsc	r0x00, 2		; bit 2
	addlw	3

	btfsc	r0x00, 3		; bit 3
	addlw	7

	btfsc	r0x00, 4		; bit 4
	addlw	5

	btfsc	r0x00, 6		; bit 6
	addlw	1

	btfsc	r0x00, 7		; bit 7
	addlw	2

	btfsc	r0x01, 0		; bit 8
	addlw	5

	btfsc	r0x01, 1		; bit 9
	addlw	1

	btfsc	r0x01, 2		; bit 10
	addlw	3

	btfsc	r0x01, 3		; bit 11
	addlw	7

	btfsc	r0x01, 4		; bit 12
	addlw	4

	btfsc	r0x01, 5		; bit 13
	addlw	8

	btfsc	r0x01, 6		; bit 14
	addlw	6

	btfsc	r0x01, 7		; bit 15
	addlw	2

	btfsc	r0x02, 0		; bit 16
	addlw	5


	clrf	d0x17
@49:
	addlw	-10
	bnc	@50
	incf	d0x17, f
	bra	@49
@50:
	addlw	10
	movwf	d0x16


	decfsz	round_digit, f
	bra	@51
	addlw	251
	bnc	@51
	incf	d0x17, f

@51:

; adding digit 17
	movf	d0x17, w

	btfsc	r0x00, 1		; bit 1
	addlw	1

	btfsc	r0x00, 2		; bit 2
	addlw	2

	btfsc	r0x00, 3		; bit 3
	addlw	4

	btfsc	r0x00, 4		; bit 4
	addlw	9

	btfsc	r0x00, 5		; bit 5
	addlw	9

	btfsc	r0x00, 6		; bit 6
	addlw	8

	btfsc	r0x00, 7		; bit 7
	addlw	6

	btfsc	r0x01, 0		; bit 8
	addlw	2

	btfsc	r0x01, 1		; bit 9
	addlw	5

	btfsc	r0x01, 4		; bit 12
	addlw	1

	btfsc	r0x01, 5		; bit 13
	addlw	2

	btfsc	r0x01, 6		; bit 14
	addlw	5

	btfsc	r0x01, 7		; bit 15
	addlw	1

	btfsc	r0x02, 0		; bit 16
	addlw	2

	btfsc	r0x02, 1		; bit 17
	addlw	5


	clrf	d0x18
@52:
	addlw	-10
	bnc	@53
	incf	d0x18, f
	bra	@52
@53:
	addlw	10
	movwf	d0x17


	decfsz	round_digit, f
	bra	@54
	addlw	251
	bnc	@54
	incf	d0x18, f

@54:

; adding digit 18
	movf	d0x18, w

	btfsc	r0x00, 5		; bit 5
	addlw	1

	btfsc	r0x00, 6		; bit 6
	addlw	3

	btfsc	r0x00, 7		; bit 7
	addlw	7

	btfsc	r0x01, 0		; bit 8
	addlw	5

	btfsc	r0x01, 2		; bit 10
	addlw	1

	btfsc	r0x01, 3		; bit 11
	addlw	2

	btfsc	r0x01, 4		; bit 12
	addlw	4

	btfsc	r0x01, 5		; bit 13
	addlw	8

	btfsc	r0x01, 6		; bit 14
	addlw	6

	btfsc	r0x01, 7		; bit 15
	addlw	3

	btfsc	r0x02, 0		; bit 16
	addlw	6

	btfsc	r0x02, 1		; bit 17
	addlw	2

	btfsc	r0x02, 2		; bit 18
	addlw	5


	clrf	d0x19
@55:
	addlw	-10
	bnc	@56
	incf	d0x19, f
	bra	@55
@56:
	addlw	10
	movwf	d0x18


	decfsz	round_digit, f
	bra	@57
	addlw	251
	bnc	@57
	incf	d0x19, f

@57:

; adding digit 19
	movf	d0x19, w

	btfsc	r0x01, 0		; bit 8
	addlw	1

	btfsc	r0x01, 1		; bit 9
	addlw	3

	btfsc	r0x01, 2		; bit 10
	addlw	6

	btfsc	r0x01, 3		; bit 11
	addlw	2

	btfsc	r0x01, 4		; bit 12
	addlw	4

	btfsc	r0x01, 5		; bit 13
	addlw	8

	btfsc	r0x01, 6		; bit 14
	addlw	7

	btfsc	r0x01, 7		; bit 15
	addlw	5

	btfsc	r0x02, 1		; bit 17
	addlw	1

	btfsc	r0x02, 2		; bit 18
	addlw	2

	btfsc	r0x02, 3		; bit 19
	addlw	5


	clrf	d0x20
@58:
	addlw	-10
	bnc	@59
	incf	d0x20, f
	bra	@58
@59:
	addlw	10
	movwf	d0x19


	decfsz	round_digit, f
	bra	@60
	addlw	251
	bnc	@60
	incf	d0x20, f

@60:

; adding digit 20
	movf	d0x20, w

	btfsc	r0x01, 3		; bit 11
	addlw	1

	btfsc	r0x01, 4		; bit 12
	addlw	2

	btfsc	r0x01, 5		; bit 13
	addlw	4

	btfsc	r0x01, 6		; bit 14
	addlw	9

	btfsc	r0x01, 7		; bit 15
	addlw	9

	btfsc	r0x02, 0		; bit 16
	addlw	9

	btfsc	r0x02, 1		; bit 17
	addlw	8

	btfsc	r0x02, 2		; bit 18
	addlw	6

	btfsc	r0x02, 3		; bit 19
	addlw	2

	btfsc	r0x02, 4		; bit 20
	addlw	5


	clrf	d0x21
@61:
	addlw	-10
	bnc	@62
	incf	d0x21, f
	bra	@61
@62:
	addlw	10
	movwf	d0x20


	decfsz	round_digit, f
	bra	@63
	addlw	251
	bnc	@63
	incf	d0x21, f

@63:

; adding digit 21
	movf	d0x21, w

	btfsc	r0x01, 7		; bit 15
	addlw	1

	btfsc	r0x02, 0		; bit 16
	addlw	3

	btfsc	r0x02, 1		; bit 17
	addlw	7

	btfsc	r0x02, 2		; bit 18
	addlw	5

	btfsc	r0x02, 3		; bit 19
	addlw	1

	btfsc	r0x02, 4		; bit 20
	addlw	2

	btfsc	r0x02, 5		; bit 21
	addlw	5


	clrf	d0x22
@64:
	addlw	-10
	bnc	@65
	incf	d0x22, f
	bra	@64
@65:
	addlw	10
	movwf	d0x21


	decfsz	round_digit, f
	bra	@66
	addlw	251
	bnc	@66
	incf	d0x22, f

@66:

; adding digit 22
	movf	d0x22, w

	btfsc	r0x02, 2		; bit 18
	addlw	1

	btfsc	r0x02, 3		; bit 19
	addlw	3

	btfsc	r0x02, 4		; bit 20
	addlw	6

	btfsc	r0x02, 5		; bit 21
	addlw	2

	btfsc	r0x02, 6		; bit 22
	addlw	5


	clrf	d0x23
@67:
	addlw	-10
	bnc	@68
	incf	d0x23, f
	bra	@67
@68:
	addlw	10
	movwf	d0x22


	decfsz	round_digit, f
	bra	@69
	addlw	251
	bnc	@69
	incf	d0x23, f

@69:

; adding digit 23
	movf	d0x23, w

	btfsc	r0x02, 5		; bit 21
	addlw	1

	btfsc	r0x02, 6		; bit 22
	addlw	2

	btfsc	r0x02, 7		; bit 23
	addlw	5


	clrf	d0x24
@70:
	addlw	-10
	bnc	@71
	incf	d0x24, f
	bra	@70
@71:
	addlw	10
	movwf	d0x23


	decfsz	round_digit, f
	bra	@72
	addlw	251
	bnc	@72
	incf	d0x24, f

@72:


; finalization
@73:
	addlw	48
	movwf	POSTINC0
	movf	d0x22, w
@74:
	addlw	48
	movwf	POSTINC0
	movf	d0x21, w
@75:
	addlw	48
	movwf	POSTINC0
	movf	d0x20, w
@76:
	addlw	48
	movwf	POSTINC0
	movf	d0x19, w
@77:
	addlw	48
	movwf	POSTINC0
	movf	d0x18, w
@78:
	addlw	48
	movwf	POSTINC0
	movf	d0x17, w
@79:
	addlw	48
	movwf	POSTINC0
	movf	d0x16, w
@80:
	addlw	48
	movwf	POSTINC0
	movf	d0x15, w
@81:
	addlw	48
	movwf	POSTINC0
	movf	d0x14, w
@82:
	addlw	48
	movwf	POSTINC0
	movf	d0x13, w
@83:
	addlw	48
	movwf	POSTINC0
	movf	d0x12, w
@84:
	addlw	48
	movwf	POSTINC0
	movf	d0x11, w
@85:
	addlw	48
	movwf	POSTINC0
	movf	d0x10, w
@86:
	addlw	48
	movwf	POSTINC0
	movf	d0x09, w
@87:
	addlw	48
	movwf	POSTINC0
	movf	d0x08, w
@88:
	addlw	48
	movwf	POSTINC0
	movf	d0x07, w
@89:
	addlw	48
	movwf	POSTINC0
	movf	d0x06, w
@90:
	addlw	48
	movwf	POSTINC0
	movf	d0x05, w
@91:
	addlw	48
	movwf	POSTINC0
	movf	d0x04, w
@92:
	addlw	48
	movwf	POSTINC0
	movf	d0x03, w
@93:
	addlw	48
	movwf	POSTINC0
	movf	d0x02, w
@94:
	addlw	48
	movwf	POSTINC0
	movf	d0x01, w
@95:
	addlw	48
	movwf	POSTINC0
	movf	d0x00, w
@96:
	addlw	48
	movwf	POSTINC0

	movlw	23

	movff	PREINC1, d0x24
	movff	PREINC1, d0x23
	movff	PREINC1, d0x22
	movff	PREINC1, d0x21
	movff	PREINC1, d0x20
	movff	PREINC1, d0x19
	movff	PREINC1, d0x18
	movff	PREINC1, d0x17
	movff	PREINC1, d0x16
	movff	PREINC1, d0x15
	movff	PREINC1, d0x14
	movff	PREINC1, d0x13
	movff	PREINC1, d0x12
	movff	PREINC1, d0x11
	movff	PREINC1, d0x10
	movff	PREINC1, d0x09
	movff	PREINC1, d0x08
	movff	PREINC1, d0x07
	movff	PREINC1, d0x06
	movff	PREINC1, d0x05
	movff	PREINC1, d0x04
	movff	PREINC1, d0x03
	movff	PREINC1, d0x02
	movff	PREINC1, d0x01
	movff	PREINC1, d0x00
	return

	end

; Converstion table
;
;00	0.5000000000000000000000000	5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
;01	0.2500000000000000000000000	2 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
;02	0.1250000000000000000000000	1 2 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
;03	0.0625000000000000000000000	0 6 2 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
;04	0.0312500000000000000000000	0 3 1 2 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
;05	0.0156250000000000000000000	0 1 5 6 2 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
;06	0.0078125000000000000000000	0 0 7 8 1 2 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
;07	0.0039062500000000000000000	0 0 3 9 0 6 2 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
;08	0.0019531250000000000000000	0 0 1 9 5 3 1 2 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
;09	0.0009765625000000000000000	0 0 0 9 7 6 5 6 2 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
;10	0.0004882812500000000000000	0 0 0 4 8 8 2 8 1 2 5 0 0 0 0 0 0 0 0 0 0 0 0 0 
;11	0.0002441406250000000000000	0 0 0 2 4 4 1 4 0 6 2 5 0 0 0 0 0 0 0 0 0 0 0 0 
;12	0.0001220703125000000000000	0 0 0 1 2 2 0 7 0 3 1 2 5 0 0 0 0 0 0 0 0 0 0 0 
;13	0.0000610351562500000000000	0 0 0 0 6 1 0 3 5 1 5 6 2 5 0 0 0 0 0 0 0 0 0 0 
;14	0.0000305175781250000000000	0 0 0 0 3 0 5 1 7 5 7 8 1 2 5 0 0 0 0 0 0 0 0 0 
;15	0.0000152587890625000000000	0 0 0 0 1 5 2 5 8 7 8 9 0 6 2 5 0 0 0 0 0 0 0 0 
;16	0.0000076293945312500000000	0 0 0 0 0 7 6 2 9 3 9 4 5 3 1 2 5 0 0 0 0 0 0 0 
;17	0.0000038146972656250000000	0 0 0 0 0 3 8 1 4 6 9 7 2 6 5 6 2 5 0 0 0 0 0 0 
;18	0.0000019073486328125000000	0 0 0 0 0 1 9 0 7 3 4 8 6 3 2 8 1 2 5 0 0 0 0 0 
;19	0.0000009536743164062500000	0 0 0 0 0 0 9 5 3 6 7 4 3 1 6 4 0 6 2 5 0 0 0 0 
;20	0.0000004768371582031250000	0 0 0 0 0 0 4 7 6 8 3 7 1 5 8 2 0 3 1 2 5 0 0 0 
;21	0.0000002384185791015625000	0 0 0 0 0 0 2 3 8 4 1 8 5 7 9 1 0 1 5 6 2 5 0 0 
;22	0.0000001192092895507812500	0 0 0 0 0 0 1 1 9 2 0 9 2 8 9 5 5 0 7 8 1 2 5 0 
;23	0.0000000596046447753906250	0 0 0 0 0 0 0 5 9 6 0 4 6 4 4 7 7 5 3 9 0 6 2 5 
