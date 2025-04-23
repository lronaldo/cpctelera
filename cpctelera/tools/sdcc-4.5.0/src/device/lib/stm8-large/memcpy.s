;--------------------------------------------------------------------------
;  memcpy.s
;
;  Copyright (C) 2018, Benedikt Freisen
;  Copyright (C) 2021, Philipp Klaus Krause
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

; This memcpy() implementation has been optimized for speed using 4x loop
; unrolling and index relative addressing.

; void *memcpy(void *dest, const void *src, size_t n);

	.globl ___memcpy
	.globl _memcpy

	.area CODE

___memcpy:
_memcpy:
	pushw	x

	ldw	y, (6, sp)

	srl	(8, sp)
	rrc	(9, sp)
	jrnc	n_x0
	ld	a, (y)
	ld	(x), a
	incw	y
	incw	x
n_x0:
	srl	(8, sp)
	rrc	(9, sp)
	jrnc	n_00
	ld	a, (y)
	ld	(x), a
	incw	y
	incw	x
	ld	a, (y)
	ld	(x), a
	incw	y
	incw	x
n_00:
	tnz	(9, sp)
	jrne	loop_ent
	dec	(8, sp)
	jrmi	end
	jra	loop_ent

loop:
	addw	y, #4
	addw	x, #4
loop_ent:
	ld	a, (y)
	ld	(x), a
	ld	a, (1, y)
	ld	(1, x), a
	ld	a, (2, y)
	ld	(2, x), a
	ld	a, (3, y)
	ld	(3, x), a

	dec	(9, sp)
	jrne	loop
	dec	(8, sp)
	jrpl	loop

end:
	popw	x
	retf

