;--------------------------------------------------------------------------
;  __ltoa.s
;
;  Copyright (C) 2020-2021, Sergey Belyashov
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
;   might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

	.area   _CODE

	.globl ___ltoa
	.globl ___ultoa
;
;void __itoa(long value, char *string, unsigned char radix);
;
___ltoa::
	push	ix
	ld	ix, #0
	add	ix, sp
;
	push	hl
;	push	de
;
;	HLDE, -4 (ix) - value
;	4 (ix) - string
;	6 (ix) - radix
;
	bit	7, h
	jr	Z, ___ultoa_de
;positive/negative numbers are supported only for radix=10
	ld	a, 6 (ix)
	cp	a, #10
	jr	NZ, ___ultoa_de
;add minus sign to result and inverse value
	ld	hl, #0
	or	a, a
	sbc	hl, de
	ex	de, hl
	ld	hl, #0
	ld	c, -2 (ix)
	ld	b, -1 (ix)
	sbc	hl, bc
	ld	-2 (ix), l
	ld	-1 (ix), h
	ld	l, 4 (ix)
	ld	h, 5 (ix)
	ld	(hl), #0x2D	;minus symbol
	inc	hl
	ld	4 (ix), l
	ld	5 (ix), h
	jr	___ultoa_dehl
;
;void __uitoa(unsigned int value, char *string, unsigned char radix);
;
___ultoa::
	push	ix
	ld	ix, #0
	add	ix, sp
;
	push	hl
;	push	de
;
;	HLDE, -4 (ix) - value
;	4 (ix) - string
;	6 (ix) - radix
;
___ultoa_de:
	ld	l, 4 (ix)
	ld	h, 5 (ix)
;
___ultoa_dehl:
	ld	a, e
	or	a, d
	or	a, -2 (ix)
	or	a, -1 (ix)
	jr	NZ, 100$
;
	ld	(hl), #0x30
	inc	hl
	jp	190$
100$:
	ld	a, 6 (ix)
	cp	a, #10		;most popular radix
	jr	NZ, 110$
;
;-------- decimal conversion
;this algorithm is 20% faster than generic one
;
	ld	c, l
	ld	b, h
	ld	hl, #-5
	add	hl, sp
	ld	sp, hl
	push	bc
	push	hl
	ld	l, -2 (ix)
	ld	h, -1 (ix)
	call	___ultobcd
	pop	de		;DE - pointer to string
	ld	hl, #0
	add	hl, sp		;HL - pointer to BCD value
	ld	b, #5		;number of bytes in BCD value
	ld	a, #0x30	;ASCII code of '0'
103$:
	rrd
	ld	(de), a
	inc	de
	rrd
	ld	(de), a
	inc	de
	inc	hl
	djnz	103$
;
;	ld	sp, hl
;skip trailing zeroes
	ld	b, #10		;real decimal number is at most 10 digits
105$:
	dec	de
	ld	a, (de)
	cp	a, #0x30
	jr	NZ, 107$	;break loop if non-zero found
	djnz	105$
107$:
	inc	de		;always point to symbol next to last significant
	ex	de, hl
	jr	190$
;
;---------------------------
;
110$:
	cp	a, #2
	jr	C, 190$		;radix is less than 2
;
	ld	c, a
	dec	c
	and	a, c
	jr	NZ, 150$
;
;-------- radix is power of 2
;
; DE - lower 16 bits of value, HL - pointer to string, C - mask
120$:
	ld	a, e
	ld	b, c
125$:
	srl	-1 (ix)
	rr	-2 (ix)
	rr	d
	rr	e
	srl	b
	jr	NZ, 125$
;
	and	a, c
	add	a, #0x30
	cp	a, #0x3A ;convert to 0...9A...Z
	jr	C, 130$
	add	a, #7
130$:
	ld	(hl), a
	inc	hl
	ld	a, e
	or	a, d
	or	a, -2 (ix)
	or	a, -1 (ix)
	jr	NZ, 120$
	jr	190$
;
;---------------------------
;
;-------- custom radix (generic algorithm)
;
150$:
	ex	de, hl
	ld	c, e
	ld	b, d
	ld	e, -2 (ix)
	ld	d, -1 (ix)
160$:
	push	bc
	ld	c, 6 (ix)
	call	___divu32_8
	pop	bc
	add	a, #0x30
	cp	a, #0x3A
	jr	C, 165$
	add	a, #7
165$:
	ld	(bc), a
	inc	bc
	ld	a, l
	or	a, h
	or	a, e
	or	a, d
	jr	NZ, 160$
	ld	l, c
	ld	h, b
;	jr	190$
;
;---------------------------
;
;-------- finish string and reverse order
190$:
	ld	(hl), #0
	ld	e, 4 (ix)
	ld	d, 5 (ix)
	call	___strreverse_reg
	ld	sp, ix
	pop	ix
	pop	hl
	inc	sp
	inc	sp
	inc	sp
	jp	(hl)
;
;in: DEHL - divident, C - divisor
;out: DEHL - quotient, A - remainder
___divu32_8:
	xor	a, a
	ld	b, #32
100$:
	add	hl, hl
	rl	e
	rl	d
	rla
	jr	c, 110$
	cp	a, c
	jr	c, 120$
110$:
	sub	a, c
	inc	l
120$:
	djnz	100$
	ret

