;--------------------------------------------------------------------------
;  _mullong.s
;
;  Copyright (C) 2014, Ben Shi
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

	.globl __mullong

	.area CODE
__mullong:
	clrw	x
	pushw	x
	pushw	x
__mullong_0:
	ld	a, (#11, sp)
	ld	xl, a
	ld	a, (#15, sp)
	mul	x, a
	ldw	(#3, sp), x
__mullong_1:
	ld	a, (#10, sp)
	ld	xl, a
	ld	a, (#15, sp)
	mul	x, a
	addw	x, (#2, sp)
	ldw	(#2, sp), x
	clr	a
	adc	a, #0
	ld	(#1, sp), a

	ld	a, (#11, sp)
	ld	xl, a
	ld	a, (#14, sp)
	mul	x, a
	addw	x, (#2, sp)
	ldw	(#2, sp), x
	clr	a
	adc	a, (#1, sp)
	ld	(#1, sp), a
__mullong_2:
	ld	a, (#11, sp)
	ld	xl, a
	ld	a, (#13, sp)
	mul	x, a
	addw	x, (#1, sp)
	ldw	(#1, sp), x

	ld	a, (#10, sp)
	ld	xl, a
	ld	a, (#14, sp)
	mul	x, a
	addw	x, (#1, sp)
	ldw	(#1, sp), x

	ld	a, (#9, sp)
	ld	xl, a
	ld	a, (#15, sp)
	mul	x, a
	addw	x, (#1, sp)
	ldw	(#1, sp), x
__mullong_3:
	ld	a, (#8, sp)
	ld	xl, a
	ld	a, (#15, sp)
	mul	x, a
	ld	a, xl
	add	a, (#1, sp)
	ld	(#1, sp), a

	ld	a, (#11, sp)
	ld	xl, a
	ld	a, (#12, sp)
	mul	x, a
	ld	a, xl
	add	a, (#1, sp)
	ld	(#1, sp), a

	ld	a, (#10, sp)
	ld	xl, a
	ld	a, (#13, sp)
	mul	x, a
	ld	a, xl
	add	a, (#1, sp)
	ld	(#1, sp), a

	ld	a, (#9, sp)
	ld	xl, a
	ld	a, (#14, sp)
	mul	x, a
	ld	a, xl
	add	a, (#1, sp)
	ld	(#1, sp), a
__mullong_4:
	popw	y
	popw	x
	retf
