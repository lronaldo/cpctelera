;--------------------------------------------------------------------------
;  memcpy.s
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

; This is a partially unrolled version of memcpy(), to reduce the count overhead.

	.globl _memcpy

	.area CODE

_memcpy:

	ldw	x, (7, sp)
	jreq	end
	ldw	y, (3, sp)

	ld	a, xl
	addw	x, #0x0807
	srlw	x
	srlw	x
	srlw	x
	exg	a, xl
	tnz	a
	exg	a, xl
	jrne	xl_nonzero
	subw	x, #0x0100
xl_nonzero:
	ldw	(7, sp), x

	ldw	x, (5, sp)
	and	a, #7
	jreq loop_8

	srl	a
	jrnc	jxx0
jxx1:
	srl	a
	jrc	jx11
jx01:
	jreq	loop_1
	jra	loop_5
jx11:
	jreq	loop_3
	jra	loop_7
jxx0:
	srl	a
	jrnc	loop_4
jx10:
	jreq	loop_2
	jra	loop_6

loop:
	incw	x
	incw	y
loop_8:
	ld	a, (x)
	ld	(y), a
	incw	x
	incw	y
loop_7:
	ld	a, (x)
	ld	(y), a
	incw	x
	incw	y
loop_6:
	ld	a, (x)
	ld	(y), a
	incw	x
	incw	y
loop_5:
	ld	a, (x)
	ld	(y), a
	incw	x
	incw	y
loop_4:
	ld	a, (x)
	ld	(y), a
	incw	x
	incw	y
loop_3:
	ld	a, (x)
	ld	(y), a
	incw	x
	incw	y
loop_2:
	ld	a, (x)
	ld	(y), a
	incw	x
	incw	y
loop_1:
	ld	a, (x)
	ld	(y), a

	dec	(8, sp)
	jrne	loop
	dec	(7, sp)
	jrne	loop

end:
	ldw	x, (3, sp)
	ret

