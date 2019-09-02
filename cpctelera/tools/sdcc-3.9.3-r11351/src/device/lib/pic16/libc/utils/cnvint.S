;--------------------------------------------------------------------------
;  cnvint.S - convertion routine of 24 bits integer to ASCII
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


POSTINC0	equ	0xfee
POSTDEC1	equ	0xfe5
PREINC1	equ	0xfe4
STATUS	equ	0xfd8


	global	_convert_int


.registers	udata_ovr	0x000
r0x00	res	1
r0x01	res	1
r0x02	res	1
r0x03	res	1
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
digit_count	res	1


S__convert_int	code
_convert_int:

	movff	r0x00, POSTDEC1
	movff	r0x01, POSTDEC1
	movff	r0x02, POSTDEC1
	movff	r0x03, POSTDEC1
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
	movff	digit_count, POSTDEC1


; adding digit 0
	movlw	0

	btfsc	r0x00, 0		; bit 0
	addlw	1

	btfsc	r0x00, 1		; bit 1
	addlw	2

	btfsc	r0x00, 2		; bit 2
	addlw	4

	btfsc	r0x00, 3		; bit 3
	addlw	8

	btfsc	r0x00, 4		; bit 4
	addlw	6

	btfsc	r0x00, 5		; bit 5
	addlw	2

	btfsc	r0x00, 6		; bit 6
	addlw	4

	btfsc	r0x00, 7		; bit 7
	addlw	8

	btfsc	r0x01, 0		; bit 8
	addlw	6

	btfsc	r0x01, 1		; bit 9
	addlw	2

	btfsc	r0x01, 2		; bit 10
	addlw	4

	btfsc	r0x01, 3		; bit 11
	addlw	8

	btfsc	r0x01, 4		; bit 12
	addlw	6

	btfsc	r0x01, 5		; bit 13
	addlw	2

	btfsc	r0x01, 6		; bit 14
	addlw	4

	btfsc	r0x01, 7		; bit 15
	addlw	8

	btfsc	r0x02, 0		; bit 16
	addlw	6

	btfsc	r0x02, 1		; bit 17
	addlw	2

	btfsc	r0x02, 2		; bit 18
	addlw	4

	btfsc	r0x02, 3		; bit 19
	addlw	8

	btfsc	r0x02, 4		; bit 20
	addlw	6

	btfsc	r0x02, 5		; bit 21
	addlw	2

	btfsc	r0x02, 6		; bit 22
	addlw	4

	btfsc	r0x02, 7		; bit 23
	addlw	8


	clrf	d0x01
@1:
	addlw	-10
	bnc	@2
	incf	d0x01, f
	bra	@1
@2:
	addlw	10
	movwf	d0x00

; adding digit 1
	movf	d0x01, w

	btfsc	r0x00, 4		; bit 4
	addlw	1

	btfsc	r0x00, 5		; bit 5
	addlw	3

	btfsc	r0x00, 6		; bit 6
	addlw	6

	btfsc	r0x00, 7		; bit 7
	addlw	2

	btfsc	r0x01, 0		; bit 8
	addlw	5

	btfsc	r0x01, 1		; bit 9
	addlw	1

	btfsc	r0x01, 2		; bit 10
	addlw	2

	btfsc	r0x01, 3		; bit 11
	addlw	4

	btfsc	r0x01, 4		; bit 12
	addlw	9

	btfsc	r0x01, 5		; bit 13
	addlw	9

	btfsc	r0x01, 6		; bit 14
	addlw	8

	btfsc	r0x01, 7		; bit 15
	addlw	6

	btfsc	r0x02, 0		; bit 16
	addlw	3

	btfsc	r0x02, 1		; bit 17
	addlw	7

	btfsc	r0x02, 2		; bit 18
	addlw	4

	btfsc	r0x02, 3		; bit 19
	addlw	8

	btfsc	r0x02, 4		; bit 20
	addlw	7

	btfsc	r0x02, 5		; bit 21
	addlw	5


	clrf	d0x02
@3:
	addlw	-10
	bnc	@4
	incf	d0x02, f
	bra	@3
@4:
	addlw	10
	movwf	d0x01

; adding digit 2
	movf	d0x02, w

	btfsc	r0x00, 7		; bit 7
	addlw	1

	btfsc	r0x01, 0		; bit 8
	addlw	2

	btfsc	r0x01, 1		; bit 9
	addlw	5

	btfsc	r0x01, 5		; bit 13
	addlw	1

	btfsc	r0x01, 6		; bit 14
	addlw	3

	btfsc	r0x01, 7		; bit 15
	addlw	7

	btfsc	r0x02, 0		; bit 16
	addlw	5

	btfsc	r0x02, 2		; bit 18
	addlw	1

	btfsc	r0x02, 3		; bit 19
	addlw	2

	btfsc	r0x02, 4		; bit 20
	addlw	5

	btfsc	r0x02, 5		; bit 21
	addlw	1

	btfsc	r0x02, 6		; bit 22
	addlw	3

	btfsc	r0x02, 7		; bit 23
	addlw	6


	clrf	d0x03
@5:
	addlw	-10
	bnc	@6
	incf	d0x03, f
	bra	@5
@6:
	addlw	10
	movwf	d0x02

; adding digit 3
	movf	d0x03, w

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
	addlw	2

	btfsc	r0x02, 0		; bit 16
	addlw	5

	btfsc	r0x02, 1		; bit 17
	addlw	1

	btfsc	r0x02, 2		; bit 18
	addlw	2

	btfsc	r0x02, 3		; bit 19
	addlw	4

	btfsc	r0x02, 4		; bit 20
	addlw	8

	btfsc	r0x02, 5		; bit 21
	addlw	7

	btfsc	r0x02, 6		; bit 22
	addlw	4

	btfsc	r0x02, 7		; bit 23
	addlw	8


	clrf	d0x04
@7:
	addlw	-10
	bnc	@8
	incf	d0x04, f
	bra	@7
@8:
	addlw	10
	movwf	d0x03

; adding digit 4
	movf	d0x04, w

	btfsc	r0x01, 6		; bit 14
	addlw	1

	btfsc	r0x01, 7		; bit 15
	addlw	3

	btfsc	r0x02, 0		; bit 16
	addlw	6

	btfsc	r0x02, 1		; bit 17
	addlw	3

	btfsc	r0x02, 2		; bit 18
	addlw	6

	btfsc	r0x02, 3		; bit 19
	addlw	2

	btfsc	r0x02, 4		; bit 20
	addlw	4

	btfsc	r0x02, 5		; bit 21
	addlw	9

	btfsc	r0x02, 6		; bit 22
	addlw	9

	btfsc	r0x02, 7		; bit 23
	addlw	8


	clrf	d0x05
@9:
	addlw	-10
	bnc	@10
	incf	d0x05, f
	bra	@9
@10:
	addlw	10
	movwf	d0x04

; adding digit 5
	movf	d0x05, w

	btfsc	r0x02, 1		; bit 17
	addlw	1

	btfsc	r0x02, 2		; bit 18
	addlw	2

	btfsc	r0x02, 3		; bit 19
	addlw	5

	btfsc	r0x02, 6		; bit 22
	addlw	1

	btfsc	r0x02, 7		; bit 23
	addlw	3


	clrf	d0x06
@11:
	addlw	-10
	bnc	@12
	incf	d0x06, f
	bra	@11
@12:
	addlw	10
	movwf	d0x05

; adding digit 6
	movf	d0x06, w

	btfsc	r0x02, 4		; bit 20
	addlw	1

	btfsc	r0x02, 5		; bit 21
	addlw	2

	btfsc	r0x02, 6		; bit 22
	addlw	4

	btfsc	r0x02, 7		; bit 23
	addlw	8


	clrf	d0x07
@13:
	addlw	-10
	bnc	@14
	incf	d0x07, f
	bra	@13
@14:
	addlw	10
	movwf	d0x06

; adding digit 7
	movf	d0x07, w

	clrf	digit_count


; finalization
	bnz	@15
	movf	d0x06, w
	bnz	@16
	movf	d0x05, w
	bnz	@17
	movf	d0x04, w
	bnz	@18
	movf	d0x03, w
	bnz	@19
	movf	d0x02, w
	bnz	@20
	movf	d0x01, w
	bnz	@21
	movf	d0x00, w
	bra	@22
@15:
	incf	digit_count, f
	addlw	48
	movwf	POSTINC0
	movf	d0x06, w
@16:
	incf	digit_count, f
	addlw	48
	movwf	POSTINC0
	movf	d0x05, w
@17:
	incf	digit_count, f
	addlw	48
	movwf	POSTINC0
	movf	d0x04, w
@18:
	incf	digit_count, f
	addlw	48
	movwf	POSTINC0
	movf	d0x03, w
@19:
	incf	digit_count, f
	addlw	48
	movwf	POSTINC0
	movf	d0x02, w
@20:
	incf	digit_count, f
	addlw	48
	movwf	POSTINC0
	movf	d0x01, w
@21:
	incf	digit_count, f
	addlw	48
	movwf	POSTINC0
	movf	d0x00, w
@22:
	incf	digit_count, f
	addlw	48
	movwf	POSTINC0

	movf	digit_count, w

	movff	PREINC1, digit_count
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
	movff	PREINC1, r0x03
	movff	PREINC1, r0x02
	movff	PREINC1, r0x01
	movff	PREINC1, r0x00
	return

	end

; Converstion table
;
;00	000000000000000000000001	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 
;01	000000000000000000000002	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 
;02	000000000000000000000004	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4 
;03	000000000000000000000008	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 8 
;04	000000000000000000000016	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 6 
;05	000000000000000000000032	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 2 
;06	000000000000000000000064	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 6 4 
;07	000000000000000000000128	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 2 8 
;08	000000000000000000000256	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 5 6 
;09	000000000000000000000512	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 5 1 2 
;10	000000000000000000001024	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 2 4 
;11	000000000000000000002048	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 0 4 8 
;12	000000000000000000004096	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4 0 9 6 
;13	000000000000000000008192	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 8 1 9 2 
;14	000000000000000000016384	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 6 3 8 4 
;15	000000000000000000032768	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 2 7 6 8 
;16	000000000000000000065536	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 6 5 5 3 6 
;17	000000000000000000131072	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 3 1 0 7 2 
;18	000000000000000000262144	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 6 2 1 4 4 
;19	000000000000000000524288	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 5 2 4 2 8 8 
;20	000000000000000001048576	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 4 8 5 7 6 
;21	000000000000000002097152	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 0 9 7 1 5 2 
;22	000000000000000004194304	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4 1 9 4 3 0 4 
;23	000000000000000008388608	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 8 3 8 8 6 0 8 
