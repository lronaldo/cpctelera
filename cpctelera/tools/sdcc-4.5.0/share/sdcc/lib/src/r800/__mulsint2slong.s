;--------------------------------------------------------------------------
;  __mulsint2slong.s
;
;  Copyright (c) 2021, Philipp Klaus Krause
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

.module u_16_16_mul

.globl ___muluint2ulong
.globl ___mulsint2slong

; uint32_t _u_16_16_mul(uint16_t l, uint16_t r);

.area _CODE

___mulsint2slong:
	; Use lowest bit of c to remember if result needs to be negated. Use b to cache #0.
	ld	bc, #0

	bit	#7, l
	jr	z, hl_nonneg
	ld	a, b
	sub	a, l
	ld	l, a
	ld	a, b
	sbc	a, h
	ld	h, a
	inc	c
hl_nonneg:

	bit	#7, e
	jr	z, de_nonneg
	ld	a, b
	sub	a, e
	ld	e, a
	ld	a, b
	sbc	a, d
	ld	d, a
	inc	c
de_nonneg:

	push	bc
	call	___muluint2ulong
	pop	bc

	bit	#0, c
	ret	z

	; Negate result.
	ld	a, b
	sub	a, e
	ld	e, a
	ld	a, b
	sbc	a, d
	ld	d, a
	ld	a, b
	sbc	a, l
	ld	l, a
	ld	a, b
	sbc	a, h
	ld	h, a
	ret

; 16x16->32 multiplication
___muluint2ulong:
	ld	iy, #0
	ld	b, #16
loop:
	add	iy, iy
	adc	hl, hl
	jr	NC, skip
	add	iy, de
	jr	NC, skip
	inc	hl
skip:
	djnz	loop
	push	iy
	pop	de
	ret

