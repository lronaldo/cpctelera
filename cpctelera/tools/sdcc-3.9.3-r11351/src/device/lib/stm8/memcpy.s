;--------------------------------------------------------------------------
;  memcpy.s
;
;  Copyright (C) 2018, Benedikt Freisen
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
	ldw	y, (3, sp)
	ldw	x, (5, sp)

	srl	(7, sp)
	rrc	(8, sp)
	jrnc	n_x0
	ld	a, (x)
	ld	(y), a
	incw	x
	incw	y
n_x0:
	srl	(7, sp)
	rrc	(8, sp)
	jrnc	n_00
	ld	a, (x)
	ld	(y), a
	incw	x
	incw	y
	ld	a, (x)
	ld	(y), a
	incw	x
	incw	y
n_00:
	tnz	(8, sp)
	jrne	loop_ent
	dec	(7, sp)
	jrmi	end
	jra	loop_ent

loop:
	addw	x, #4
	addw	y, #4
loop_ent:
	ld	a, (x)
	ld	(y), a
	ld	a, (1, x)
	ld	(1, y), a
	ld	a, (2, x)
	ld	(2, y), a
	ld	a, (3, x)
	ld	(3, y), a

	dec	(8, sp)
	jrne	loop
	dec	(7, sp)
	jrpl	loop

end:
	ldw	x, (3, sp)
	ret

