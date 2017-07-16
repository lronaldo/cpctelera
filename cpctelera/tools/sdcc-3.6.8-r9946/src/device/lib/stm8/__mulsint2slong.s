;--------------------------------------------------------------------------
;  __mulsint2slong.s
;
;  Copyright (C) 2016, Philipp Klaus Krause
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

.globl ___mulsint2slong
.globl ___muluint2ulong

.area CODE

___muluint2ulong:
    clr a
    ldw x, (5, sp)
    jra right_nonneg

___mulsint2slong:
	
	; Handle signed operands
	clr	a
	ldw	x, (3, sp)
	jrpl	left_nonneg
	cpl	a
	negw	x
	ldw	(3, sp), x
left_nonneg:
	ldw	x, (5, sp)
	jrpl	right_nonneg
	cpl	a
	negw	x
	ldw	(5, sp), x
right_nonneg:

	sub	sp, #4
	push	a

	; Multiply lower bytes
	ld	a, (5+4, sp)
	mul	x, a
	ldw	(4, sp), x

	; Multiply upper bytes
	ldw	x, (5+2, sp)
	ld	a, (5+5, sp)
	mul	x, a
	ldw	(2, sp), x

	; Multiply middle bytes
	ld	a, (5+5, sp)
	jreq	skip_m1
	ldw	x, (5+3, sp)
	mul	x, a
	addw	x, (3, sp)
	ldw	(3, sp), x
	ld	a, (2, sp)
	adc	a, #0
	ld	(2, sp), a
skip_m1:

	ld	a, (5+3, sp)
	jreq	skip_m2
	ldw	x, (5+5, sp)
	mul	x, a
	addw	x, (3, sp)
	ldw	(3, sp), x
	ld	a, (2, sp)
	adc	a, #0
	ld	(2, sp), a
skip_m2:

	; Handle signed result
	pop	a
	popw	y
	popw	x
	tnz	a
jrpl	end
	negw	x
jrnc	neg_y
	incw	y
neg_y:
	negw	y
end:
	ret

